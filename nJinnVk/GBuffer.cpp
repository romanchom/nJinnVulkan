#include "stdafx.hpp"
#include "GBuffer.hpp"

#include "Context.hpp"
#include "Screen.hpp"
#include "CommandBuffer.hpp"
#include "RendererSystem.hpp"
#include "DescriptorSet.hpp"

namespace nJinn {
	static const vk::Format formats[] =
	{
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eA2B10G10R10UnormPack32,
		vk::Format::eA2B10G10R10UnormPack32,
		vk::Format::eB8G8R8A8Srgb, // temporary proof of concept
		//vk::Format::eR16G16B16A16Sfloat,
	};

	GBuffer::GBuffer() {}

	void GBuffer::initialize(uint32_t width, uint32_t height)
	{
		vk::ImageCreateInfo imageInfo;

		// depth stencil buffer
		imageInfo
			.setImageType(vk::ImageType::e2D)
			.setFormat(formats[depthStencilAttachmentIndex])
			.setExtent(vk::Extent3D(width, height, 1))
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

		imageInfo.setFormat(formats[hdrColorAttachmentIndex]);
		mGBufferImages[hdrColorAttachmentIndex] = context->dev().createImage(imageInfo);

		vk::DeviceSize totalSizeRequired = 0;
		vk::DeviceSize offsets[renderPassAttachmentsCount];

		// temporary proof of concept
		for (int i = 0; i < renderPassAttachmentsCount; ++i) {
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
		for (int i = 0; i < renderPassAttachmentsCount; ++i) {
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
				{},
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

		vk::CommandBuffer cmdbuf = cmdBuf.get();

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmdbuf);

		context->mainQueue().submit(1, &submitInfo, nullptr);
		context->mainQueue().waitIdle();

		vk::ImageView images[renderPassAttachmentsCount];
		for (int i = 0; i < renderPassAttachmentsCount - 1; ++i) {
			images[i] = mImageViews[i];
		}

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setRenderPass(rendererSystem->mDeferredRenderPass)
			.setAttachmentCount(renderPassAttachmentsCount)
			.setPAttachments(images)
			.setWidth(screen->width())
			.setHeight(screen->height())
			.setLayers(1);

		for (int i = 0; i < 2; ++i) {
			images[hdrColorAttachmentIndex] = screen->getImageView(i);
			mFramebuffers[i] = context->dev().createFramebuffer(framebufferInfo);
		}
	}

	void GBuffer::writeDescriptorSet(DescriptorSet & descriptorSet)
	{
		vk::DescriptorImageInfo imageInfos[renderPassAttachmentsCount];
		imageInfos[0]
			.setImageLayout(vk::ImageLayout::eGeneral)
			.setImageView(mDepthOnlyImageView);
		for (int i = 1; i < renderPassAttachmentsCount; ++i) {
			imageInfos[i]
				.setImageView(mImageViews[i])
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		DescriptorWriter(descriptorSet).attachment(imageInfos, 0, 4);
	}

	vk::Framebuffer GBuffer::framebuffer() {
		return mFramebuffers[screen->currentFrameIndex()]; 
	}
	

	GBuffer::~GBuffer()
	{
		for (int i = 0; i < 2; ++i) {
			context->dev().destroyFramebuffer(mFramebuffers[i]);
		}

		for (int i = 0; i < renderPassAttachmentsCount; ++i) {
			context->dev().destroyImageView(mImageViews[i]);
			context->dev().destroyImage(mGBufferImages[i]);
		}
	}
}
