#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"
#include "Debug.hpp"
#include "ResourceManager.hpp"
#include "Camera.hpp"
#include "ResourceUploader.hpp"

namespace nJinn {
	RendererSystem * rendererSystem;

	static const vk::Format formats[RendererSystem::renderPassAttachmentsCount] =
	{
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eR16G16B16A16Sfloat,
		vk::Format::eR16G16Snorm,
		vk::Format::eB8G8R8A8Srgb, // temporary proof of concept
		//vk::Format::eR16G16B16A16Sfloat,
	};

	void RendererSystem::createRenderPass()
	{
		// --------------------------
		// GEOMETRY PASS attachments  
		// diffuse color
		// normal
		// specular etc.
		vk::AttachmentDescription renderPassAttachments[renderPassAttachmentsCount];
		
		// depth stencil
		renderPassAttachments[depthStencilAttachmentIndex]
			.setFormat(formats[depthStencilAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
			.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		// diffuse color
		renderPassAttachments[gBufferDiffuseColorAttachmentIndex]
			.setFormat(formats[gBufferDiffuseColorAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// normal in compressed RG format + 10 bit specular in B
		renderPassAttachments[gBufferNormalSpecularAttachmentIndex]
			.setFormat(formats[gBufferNormalSpecularAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// hdr output color buffer
		renderPassAttachments[hdrColorAttachmentIndex]
			.setFormat(formats[hdrColorAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);


		// SUBPASSES ----------------------------------
		vk::SubpassDescription subpasses[subpassCount];
		// geometry subpass
		vk::AttachmentReference geometrySubpassAttachments[] = 
		{ 
			{ gBufferDiffuseColorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal },
			{ gBufferNormalSpecularAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal },
			{ depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal },
		};

		subpasses[geometrySubpassIndex]
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(geometrySubpassColorAttachmentsCount)
			.setPColorAttachments(geometrySubpassAttachments)
			.setPDepthStencilAttachment(geometrySubpassAttachments + geometrySubpassColorAttachmentsCount);
		
		// lighting subpass
		vk::AttachmentReference hdrColorAttachment(hdrColorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
		//vk::AttachmentReference lightingDepthStencilAttachment(depthStencilAttachmentIndex, vk::ImageLayout::eGeneral);

		vk::AttachmentReference lightingInputAttachments[] =
		{
			{ gBufferDiffuseColorAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ gBufferNormalSpecularAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ depthStencilAttachmentIndex, vk::ImageLayout::eGeneral },
		};

		subpasses[lightingSubpassIndex]
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setInputAttachmentCount(3)
			.setPInputAttachments(lightingInputAttachments)
			.setColorAttachmentCount(1) // only one output
			.setPColorAttachments(&hdrColorAttachment)
			.setPDepthStencilAttachment(lightingInputAttachments + 2);

		// SUBPASS DEPENDENCIES -----------------
		vk::SubpassDependency dependencies[2];
		dependencies[0]
			.setSrcSubpass(geometrySubpassIndex)
			.setDstSubpass(lightingSubpassIndex)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eColorAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead
				| vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests
				| vk::PipelineStageFlagBits::eFragmentShader
				| vk::PipelineStageFlagBits::eLateFragmentTests);

		dependencies[1]
			.setSrcSubpass(lightingSubpassIndex)
			.setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eColorAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead
				| vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests
				| vk::PipelineStageFlagBits::eFragmentShader
				| vk::PipelineStageFlagBits::eLateFragmentTests);

		// RENDER PASS ------------------
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo
			.setAttachmentCount(renderPassAttachmentsCount)
			.setPAttachments(renderPassAttachments)
			.setSubpassCount(subpassCount)
			.setPSubpasses(subpasses)
			.setDependencyCount(2) 
			.setPDependencies(dependencies);
		
		mDeferredRenderPass = context->dev().createRenderPass(renderPassInfo);
	}

	RendererSystem::RendererSystem() :
		mCurrentSyncIndex(0)
	{
		createRenderPass();
		//mGlobalUniforms.initialize(sizeof(GlobalUniformsStruct));

		vk::DescriptorSetLayoutBinding geometryBindings[] = {
			{0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eAllGraphics }
		};
		mGeometryDescriptorAllocator.initialize(geometryBindings, 1);
		vk::DescriptorSetLayout descSetLayouts[] = {
			mGeometryDescriptorAllocator.layout()
		};

		vk::DescriptorSetLayoutBinding lightingBindings[] = {
			{ 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
			{ 1, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
			{ 2, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
		};
		mLightingDescriptorAllocator.initialize(lightingBindings, 3);

		vk::PipelineLayoutCreateInfo info;
		info
			.setPSetLayouts(descSetLayouts)
			.setSetLayoutCount(1);
		mGeometryPipelineLayout = context->dev().createPipelineLayout(info);

		descSetLayouts[0] = mLightingDescriptorAllocator.layout();
		mLightingPipelineLayout = context->dev().createPipelineLayout(info);

		for (int i = 0; i < 2; ++i) {
			mSyncs.emplace_back(i == 0);
		}
	}

	RendererSystem::~RendererSystem()
	{
		context->dev().destroyPipelineLayout(mLightingPipelineLayout);
		context->dev().destroyPipelineLayout(mGeometryPipelineLayout);
		context->dev().destroyRenderPass(mDeferredRenderPass);
	}

	void RendererSystem::update()
	{
		auto fence = mSyncs[mCurrentSyncIndex].renderingCompleteFence.get();
		while (vk::Result::eTimeout == context->dev()
			.waitForFences(1, &fence, true, UINT64_MAX));
		context->dev().resetFences(1, &fence);

		++mCurrentSyncIndex %= mSyncs.size();

		Sync & sync = mSyncs[mCurrentSyncIndex];
		screen->acquireFrame(sync.frameAcquiredSemaphore.get());

		//mGlobalUniforms.update();
		//GlobalUniformsStruct * unis = mGlobalUniforms.acquire<GlobalUniformsStruct>();

		for (auto && obj : mDeferredObjects) {
			obj->update();
		}

		mCommandBuffersToExecute.clear();
		int i = 0;
		for (auto && camera : mCameras) {
			camera->draw(mDeferredObjects, mLightSources);
			mCommandBuffersToExecute.push_back(camera->mCommandBuffer.get());
			++i;
		}


		vk::Semaphore waitSemaphores[] = {
			sync.frameAcquiredSemaphore.get(),
			resourceUploader->semaphore()
		};
		vk::Semaphore signalSemaphores[] = {
			sync.renderingCompleteSemaphore.get() 
		};

		vk::PipelineStageFlags src[] = {
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eVertexShader,
		};

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(static_cast<uint32_t>(rendererSystem->mCommandBuffersToExecute.size()))
			.setPCommandBuffers(rendererSystem->mCommandBuffersToExecute.data())
			.setPWaitSemaphores(waitSemaphores)
			.setWaitSemaphoreCount(2)
			.setPWaitDstStageMask(src)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(signalSemaphores);

		context->mainQueue().submit(1, &submitInfo, sync.renderingCompleteFence.get());

		screen->present(sync.renderingCompleteSemaphore.get());
	}
}
