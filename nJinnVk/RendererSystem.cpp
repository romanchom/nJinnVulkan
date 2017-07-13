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

namespace nJinn {
	RendererSystem * rendererSystem;

	static const vk::Format formats[RendererSystem::renderPassAttachmentsCount] =
	{
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eA2B10G10R10UnormPack32,
		vk::Format::eA2B10G10R10UnormPack32,
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

	RendererSystem::RendererSystem()
	{
		createRenderPass();
		mGlobalUniforms.initialize(sizeof(GlobalUniformsStruct));

		vk::DescriptorSetLayoutBinding geometryBindings[] = {
			{0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eAllGraphics }
		};
		mGeometryDescriptorAllocator.initialize(geometryBindings, 1);
		vk::DescriptorSetLayout descSetLayouts[] = {
			mGeometryDescriptorAllocator.layout()
		};

		vk::DescriptorSetLayoutBinding lightingBindings[] = {
			{ 0, vk::DescriptorType::eInputAttachment, 4,  vk::ShaderStageFlagBits::eFragment }
		};
		mLightingDescriptorAllocator.initialize(lightingBindings, 1);

		vk::PipelineLayoutCreateInfo info;
		info
			.setPSetLayouts(descSetLayouts)
			.setSetLayoutCount(1);
		mGeometryPipelineLayout = context->dev().createPipelineLayout(info);

		descSetLayouts[0] = mLightingDescriptorAllocator.layout();
		mLightingPipelineLayout = context->dev().createPipelineLayout(info);
	}

	RendererSystem::~RendererSystem()
	{
		context->dev().destroyRenderPass(mDeferredRenderPass);
	}

	void RendererSystem::update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsC)
	{
		mGlobalUniforms.update();
		GlobalUniformsStruct * unis = mGlobalUniforms.acquire<GlobalUniformsStruct>();

		vk::PipelineStageFlags src[] = {
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
		};

		vk::CommandBuffer cmdBuffs[10];
		int i = 0;
		for (auto && camera : mCameras) {
			camera->draw(mDeferredObjects, mLightSources);
			cmdBuffs[i] = camera->mCommandBuffer.get();
			++i;
		}

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(i)
			.setPCommandBuffers(cmdBuffs)
			.setPWaitSemaphores(wSems)
			.setWaitSemaphoreCount(wSemC)
			.setPWaitDstStageMask(src)
			.setSignalSemaphoreCount(sSemsC)
			.setPSignalSemaphores(sSems);

		context->mainQueue().submit(1, &submitInfo, nullptr);
	}
}
