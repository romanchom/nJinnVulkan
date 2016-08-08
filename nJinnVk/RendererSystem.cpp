#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"

namespace nJinn {
	RendererSystem * rendererSystem;

	static enum {

		depthStencilAttachmentIndex = 0,
		gBufferDiffuseColorAttachmentIndex,
		gBufferNormalSpecularAttachmentIndex,
		hdrColorAttachmentIndex,
		renderPassAttachmentsCount,

		
		geometrySubpassColorAttachmentsCount = 3,
		
		geometrySubpassIndex = 0,
		lightingSubpassIndex,
		subpassCount,
		// DO NOT USE
		forwardSubpassIndex,
		postprocessIndex,

		subpassCount = 2,

	};

	void RendererSystem::createRenderPass()
	{
		// --------------------------
		// GEOMETRY PASS attachments BEGIN  
		// diffuse color
		// normal
		// specular etc.
		
		// depth stencil
		vk::AttachmentDescription renderPassAttachments[renderPassAttachmentsCount];
		renderPassAttachments[depthStencilAttachmentIndex]
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
			.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		// diffuse color
		renderPassAttachments[gBufferDiffuseColorAttachmentIndex]
			.setFormat(vk::Format::eA2R10G10B10SnormPack32)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// normal in compressed RG format + 10 bit specular in B
		renderPassAttachments[gBufferNormalSpecularAttachmentIndex]
			.setFormat(vk::Format::eA2R10G10B10SnormPack32)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// hdr output color buffer
		renderPassAttachments[hdrColorAttachmentIndex]
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentReference geometrySubpassAttachments[] = 
		{ 
			{ gBufferDiffuseColorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal },
			{ gBufferNormalSpecularAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal },
			{ depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal },
		};

		vk::SubpassDescription subpasses[subpassCount];
		subpasses[geometrySubpassIndex]
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(geometrySubpassColorAttachmentsCount)
			.setPColorAttachments(geometrySubpassAttachments)
			.setPDepthStencilAttachment(geometrySubpassAttachments + geometrySubpassColorAttachmentsCount);
		
		vk::AttachmentReference hdrColorAttachment(hdrColorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
		vk::AttachmentReference lightingDepthStencilAttachment(depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal);


		vk::AttachmentReference lightingInputAttachments[] =
		{
			{ gBufferDiffuseColorAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ gBufferNormalSpecularAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal },
		};

		subpasses[lightingSubpassIndex]
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setInputAttachmentCount(2)
			.setPInputAttachments(lightingInputAttachments)
			.setColorAttachmentCount(1) // only one output
			.setPColorAttachments(&hdrColorAttachment)
			.setPDepthStencilAttachment(&lightingDepthStencilAttachment);

		vk::SubpassDependency dependencies[2];
		dependencies[0]
			.setSrcSubpass(geometrySubpassIndex)
			.setDstSubpass(lightingSubpassIndex)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead
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
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentRead
				| vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests
				| vk::PipelineStageFlagBits::eFragmentShader
				| vk::PipelineStageFlagBits::eLateFragmentTests);


		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo
			.setAttachmentCount(renderPassAttachmentsCount)
			.setPAttachments(renderPassAttachments)
			.setSubpassCount(subpassCount)
			.setPSubpasses(subpasses)
			.setDependencyCount(2)
			.setPDependencies(dependencies);
		

	}

	RendererSystem::RendererSystem()
	{}

	RendererSystem::~RendererSystem()
	{}

	void RendererSystem::update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsC)
	{
		cmdbuf.beginRecording();
		screen->transitionForDraw(cmdbuf);
		

		uint32_t asd[] = { 0, 0, screen->width(), screen->height() };
		vk::Rect2D rendArea;
		memcpy(&rendArea, asd, 16);

		float vals[] = { 0.1f, 0.1f, 0.0f, 1.0f };
		vk::ClearValue val;
		memcpy(&val, vals, 16);

		vk::Viewport view;
		view
			.setWidth((float) screen->width())
			.setHeight((float) screen->height())
			.setMinDepth(0)
			.setMaxDepth(1)
			.setX(0)
			.setY(0);

		vk::RenderPassBeginInfo info;
		info
			.setRenderPass(screen->renderPass())
			.setFramebuffer(screen->framebuffer())
			.setRenderArea(rendArea)
			.setClearValueCount(1)
			.setPClearValues(&val);

		cmdbuf->beginRenderPass(info, vk::SubpassContents::eInline);
		cmdbuf->setViewport(0, 1, &view);
		cmdbuf->setScissor(0, 1, &rendArea);

		// TODO do this on separate thread or something
		for (auto rend : mRenderersSet) {
			rend->update();
		}

		for (auto rend : mRenderersSet) {
			rend->draw(cmdbuf);
		}


		cmdbuf->endRenderPass();
		screen->transitionForPresent(cmdbuf);
		cmdbuf.endRecording();

		vk::PipelineStageFlags src[] = {
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
		};

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(cmdbuf.get())
			.setPWaitSemaphores(wSems)
			.setWaitSemaphoreCount(wSemC)
			.setPWaitDstStageMask(src)
			.setSignalSemaphoreCount(sSemsC)
			.setPSignalSemaphores(sSems);

		context->mainQueue().submit(1, &submitInfo, nullptr);
	}
}
