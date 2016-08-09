#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"
#include "Debug.hpp"
#include "ResourceManager.hpp"

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
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// normal in compressed RG format + 10 bit specular in B
		renderPassAttachments[gBufferNormalSpecularAttachmentIndex]
			.setFormat(formats[gBufferNormalSpecularAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// hdr output color buffer
		renderPassAttachments[hdrColorAttachmentIndex]
			.setFormat(formats[hdrColorAttachmentIndex])
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
			//.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);


		// SUBPASSES ----------------------------------
		// geometry subpass
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
		

		// lighting subpass
		vk::AttachmentReference hdrColorAttachment(hdrColorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
		vk::AttachmentReference lightingDepthStencilAttachment(depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentReference lightingInputAttachments[] =
		{
			{ gBufferDiffuseColorAttachmentIndex, vk::ImageLayout::eGeneral },
			{ gBufferNormalSpecularAttachmentIndex, vk::ImageLayout::eGeneral },
		};

		subpasses[lightingSubpassIndex]
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setInputAttachmentCount(2)
			.setPInputAttachments(lightingInputAttachments)
			.setColorAttachmentCount(1) // only one output
			.setPColorAttachments(&hdrColorAttachment)
			.setPDepthStencilAttachment(&lightingDepthStencilAttachment);


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
			//.setDependencyCount(1) // temporary proof of concept
			.setDependencyCount(2) 
			.setPDependencies(dependencies);
		
		mDeferredRenderPass = context->dev().createRenderPass(renderPassInfo);
	}

	void RendererSystem::createGBuffer()
	{
		vk::ImageCreateInfo imageInfo;

		// depth stencil buffer
		imageInfo
			.setImageType(vk::ImageType::e2D)
			.setFormat(formats[depthStencilAttachmentIndex])
			.setExtent(vk::Extent3D(screen->width(), screen->height(), 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment
				| vk::ImageUsageFlagBits::eInputAttachment)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		mGBufferImages[depthStencilAttachmentIndex] = context->dev().createImage(imageInfo);

		imageInfo
			.setFormat(formats[gBufferDiffuseColorAttachmentIndex])
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment
				| vk::ImageUsageFlagBits::eInputAttachment);
		mGBufferImages[gBufferDiffuseColorAttachmentIndex] = context->dev().createImage(imageInfo);
		
		imageInfo.setFormat(formats[gBufferNormalSpecularAttachmentIndex]);
		mGBufferImages[gBufferNormalSpecularAttachmentIndex] = context->dev().createImage(imageInfo);
		
		//imageInfo.setFormat(formats[hdrColorAttachmentIndex]);
		//mGBufferImages[hdrColorAttachmentIndex] = context->dev().createImage(imageInfo);
		
		uint32_t totalSizeRequired = 0;
		uint32_t offsets[renderPassAttachmentsCount];

		// temporary proof of concept
		for (int i = 0; i < renderPassAttachmentsCount - 1; ++i) {
			vk::MemoryRequirements memReq = context->dev().getImageMemoryRequirements(mGBufferImages[i]);
			totalSizeRequired += memReq.alignment - 1;
			totalSizeRequired /= memReq.alignment;
			totalSizeRequired *= memReq.alignment;
			offsets[i] = totalSizeRequired;
			totalSizeRequired += memReq.size;
		}
		mGBufferMemory.allocate(totalSizeRequired);
		
		vk::ImageSubresourceRange range(
			vk::ImageAspectFlagBits::eDepth
			| vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);

		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo
			.setViewType(vk::ImageViewType::e2D)
			.setSubresourceRange(range);

		vk::ImageMemoryBarrier defineLayoutBarrier;
		defineLayoutBarrier
			.setSubresourceRange(range)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

		CommandBuffer cmdBuf;
		cmdBuf.beginRecording();

		// temporary proof of concept
		for (int i = 0; i < renderPassAttachmentsCount - 1/* <- fix this -1*/; ++i) {
			context->dev().bindImageMemory(mGBufferImages[i],
				mGBufferMemory.deviceMemory(),
				mGBufferMemory.offset() + offsets[i]);

			imageViewInfo
				.setImage(mGBufferImages[i])
				.setFormat(formats[i]);
			
			mImageViews[i] = context->dev().createImageView(imageViewInfo);
			
			defineLayoutBarrier.setImage(mGBufferImages[i]);
			cmdBuf->pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::DependencyFlags(),
				0, nullptr,
				0, nullptr,
				1, &defineLayoutBarrier);

			range.setAspectMask(vk::ImageAspectFlagBits::eColor);
			imageViewInfo.setSubresourceRange(range);
			defineLayoutBarrier
				.setSubresourceRange(range)
				.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		range.setAspectMask(vk::ImageAspectFlagBits::eDepth);
		imageViewInfo
			.setSubresourceRange(range)
			.setImage(mGBufferImages[0])
			.setFormat(formats[0]);
		mDepthOnlyImageView = context->dev().createImageView(imageViewInfo);

		cmdBuf.endRecording();

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(cmdBuf.get());

		context->mainQueue().submit(1, &submitInfo, nullptr);
		context->mainQueue().waitIdle();

	}

	void RendererSystem::createFramebuffer()
	{
		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setRenderPass(mDeferredRenderPass)
			.setAttachmentCount(renderPassAttachmentsCount)
			.setPAttachments(mImageViews)
			.setWidth(screen->width())
			.setHeight(screen->height())
			.setLayers(1);
		
		for (int i = 0; i < 2; ++i) {
			mImageViews[hdrColorAttachmentIndex] = screen->getImageView(i);
			mFramebuffers[i] = context->dev().createFramebuffer(framebufferInfo);
		}
	}

	RendererSystem::RendererSystem()
	{
		createRenderPass();
		createGBuffer();
		createFramebuffer();

		mat = resourceManager->get<MaterialFamily>("quad.yml", true);
		mesh = resourceManager->get<Mesh>("quad.vbm", true);

		pipe = pipelineFactory->createPipeline(*mat, *mesh, mDeferredRenderPass, lightingSubpassIndex);
		mDescSet = mat->mObjectAllocator.allocateDescriptorSet();

		//uniforms.initialize(16);

		vk::DescriptorImageInfo imageInfos[renderPassAttachmentsCount];
		imageInfos[0]
			.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setImageView(mDepthOnlyImageView);
		for (int i = 1; i < renderPassAttachmentsCount; ++i) {
			imageInfos[i]
				.setImageView(mImageViews[i])
				.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		vk::WriteDescriptorSet descWrite;
		descWrite
			.setDescriptorCount(2)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(0)
			.setDstSet(mDescSet)
			.setPImageInfo(imageInfos);

		context->dev().updateDescriptorSets(1, &descWrite, 0, nullptr);
	}

	RendererSystem::~RendererSystem()
	{
		for (int i = 0; i < 2; ++i) {
			context->dev().destroyFramebuffer(mFramebuffers[i]);
		}
		context->dev().destroyPipeline(pipe);
		for (int i = 0; i < renderPassAttachmentsCount; ++i) {
			context->dev().destroyImage(mGBufferImages[i]);
			context->dev().destroyImageView(mImageViews[i]);
		}
		context->dev().destroyRenderPass(mDeferredRenderPass);
	}

	void RendererSystem::update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsC)
	{

		vk::Rect2D rendArea;
		rendArea.extent.setWidth(screen->width()).setHeight(screen->height());

		vk::ClearValue vals[renderPassAttachmentsCount];

		for (int i = 0; i < renderPassAttachmentsCount; ++i) {
			vals[i].color.setFloat32({0.1f, 0.1f, 0.1f, 0.1f});
		}
		vals[depthStencilAttachmentIndex].depthStencil.setDepth(0.0f).setStencil(0);

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
			.setRenderPass(mDeferredRenderPass)
			.setFramebuffer(mFramebuffers[screen->currentFrameIndex()])
			.setRenderArea(rendArea)
			.setClearValueCount(4)
			.setPClearValues(vals);

		// TODO do this on separate thread or something
		for (auto rend : mRenderersSet) {
			rend->update();
		}

		cmdbuf.beginRecording();
		screen->transitionForDraw(cmdbuf);
		cmdbuf->beginRenderPass(info, vk::SubpassContents::eInline);
		// geometry pass

		cmdbuf->setViewport(0, 1, &view);
		cmdbuf->setScissor(0, 1, &rendArea);
		for (auto rend : mRenderersSet) {
			rend->draw(cmdbuf);
		}

		cmdbuf->nextSubpass(vk::SubpassContents::eInline);
		// lighting pass

		cmdbuf->bindPipeline(vk::PipelineBindPoint::eGraphics, pipe);
		cmdbuf->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mat->layout(), 1, 1, &mDescSet, 0, nullptr);
		mesh->bind(cmdbuf);
		mesh->draw(cmdbuf);

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
