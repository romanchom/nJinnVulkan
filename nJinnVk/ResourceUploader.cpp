#include "stdafx.hpp"
#include "ResourceUploader.hpp"

#include "Context.hpp"
#include "Math.hpp"
#include "Image.hpp"

namespace nJinn {
	StagingBuffer::StagingBuffer(vk::DeviceSize size) {
		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		mBuffer = context->dev().createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = context->dev().getBufferMemoryRequirements(mBuffer);

		mMemory = memory->upload().alloc(memReq.size);

		context->dev().bindBufferMemory(mBuffer, mMemory.memory(), mMemory.offset());
	}

	StagingBuffer::~StagingBuffer() {
		if (nullptr != mBuffer) {
			context->dev().destroyBuffer(mBuffer);
			memory->upload().free(mMemory);
		}
	}

	ResourceUploader::ResourceUploader() :
		mMainCommandBuffer(context->transferQueueFamilyIndex()),
		mCopyCommandBuffer(context->transferQueueFamilyIndex()),
		mReleaseOwnershipBuffer(context->transferQueueFamilyIndex()),
		mTakeOwnershipBuffer(context->mainQueueFamilyIndex()),
		mCurrentIndex(0),
		mTasksAdded(false),
		mResourcesAvailable(false)
	{
		mCopyCommandBuffer.beginRecording();
	}

	ResourceUploader::~ResourceUploader() {}

	void ResourceUploader::uploadBuffer(StagingBuffer source, vk::Buffer destination) {
		vk::BufferCopy region;
		region
			.setSrcOffset(0)
			.setDstOffset(0)
			.setSize(source.size());

		vk::BufferMemoryBarrier * initBarrier = nullptr;
		vk::BufferMemoryBarrier * ownershipBarrier = nullptr;
		{
			std::lock_guard<std::mutex> lock(mMutex);

			mBufferInitialBarriers.emplace_back();
			initBarrier = &mBufferInitialBarriers.back();

			mBufferOwnershipBarriers.emplace_back();
			ownershipBarrier = &mBufferOwnershipBarriers.back();

			mCopyCommandBuffer->copyBuffer(
				source.buffer(),
				destination,
				1, &region);
		}

		(*initBarrier)
			.setBuffer(destination)
			.setSrcAccessMask({})
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSize(source.size());

		(*ownershipBarrier)
			.setBuffer(destination)
			.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setDstAccessMask(vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eIndexRead)
			.setSrcQueueFamilyIndex(context->transferQueueFamilyIndex())
			.setDstQueueFamilyIndex(context->mainQueueFamilyIndex())
			.setSize(source.size());

		mStagingBuffers[mCurrentIndex].emplace_back(std::move(source));
		mTasksAdded = true;
	}

	void ResourceUploader::uploadImage(StagingBuffer source, Image & destination) {
		uint32_t width = destination.width();
		uint32_t height = destination.height();
		uint32_t offset = 0;

		uint32_t blockWidth = 1;
		uint32_t blockHeight = 1;
		uint32_t blockSize = 1;
		if (destination.format() == vk::Format::eBc7SrgbBlock) {
			blockHeight = 4;
			blockWidth = 4;
			blockSize = 16;
		} else {
			throw std::runtime_error("Too lazy");
		}

		auto copies = std::make_unique<vk::BufferImageCopy[]>(destination.mipLevels());

		for (uint32_t m = 0; m < destination.mipLevels(); ++m) {
			auto & copy = copies[m];
			copy.imageExtent
				.setWidth(nextMultipleOf(width, blockWidth))
				.setHeight(nextMultipleOf(height, blockHeight));
			copy.imageSubresource
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(m);
			copy
				.setBufferOffset(offset);

			width = (width + 1) / 2;
			height = (height + 1) / 2;
		}

		vk::ImageMemoryBarrier * initBarrier = nullptr;
		vk::ImageMemoryBarrier * ownershipBarrier = nullptr;
		{
			std::lock_guard<std::mutex> lock(mMutex);

			mImageInitialBarriers.emplace_back();
			initBarrier = &mImageInitialBarriers.back();

			mImageOwnershipBarriers.emplace_back();
			ownershipBarrier = &mImageOwnershipBarriers.back();

			mCopyCommandBuffer->copyBufferToImage(
				source.buffer(),
				destination.image(),
				vk::ImageLayout::eTransferDstOptimal,
				destination.mipLevels(), copies.get());
		}

		(*initBarrier)
			.setImage(destination.image())
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.subresourceRange
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setBaseMipLevel(0)
			.setLevelCount(destination.mipLevels());

		(*ownershipBarrier)
			.setImage(destination.image())
			.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
			.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setSrcQueueFamilyIndex(context->transferQueueFamilyIndex())
			.setDstQueueFamilyIndex(context->mainQueueFamilyIndex())
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.subresourceRange
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setBaseMipLevel(0)
			.setLevelCount(destination.mipLevels());

		mStagingBuffers[mCurrentIndex].emplace_back(std::move(source));
		mTasksAdded = true;
	}

	void ResourceUploader::execute() {
		++mCurrentIndex %= 2;
		mResourcesAvailable = false;
		if (mTasksAdded) {

			mCopyCommandBuffer.endRecording();

			// begin recording all transfer commands
			mMainCommandBuffer.beginRecording();
			// transition resources into correct layout
			mMainCommandBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer,
				{},
				0, nullptr,
				static_cast<uint32_t>(mBufferInitialBarriers.size()),
				mBufferInitialBarriers.data(),
				static_cast<uint32_t>(mImageInitialBarriers.size()),
				mImageInitialBarriers.data()
			);
			mMainCommandBuffer.endRecording();

			// release ownership of resources
			mReleaseOwnershipBuffer.beginRecording();
			mReleaseOwnershipBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				// actually this parameter is ignored, but validation layers are complaining
				vk::PipelineStageFlagBits::eBottomOfPipe,
				{},
				0, nullptr,
				static_cast<uint32_t>(mBufferOwnershipBarriers.size()),
				mBufferOwnershipBarriers.data(),
				static_cast<uint32_t>(mImageOwnershipBarriers.size()),
				mImageOwnershipBarriers.data()
			);
			// end recording
			mReleaseOwnershipBuffer.endRecording();

			// execute all transfer commands on transfer queue
			vk::CommandBuffer buffers[] = {
				mMainCommandBuffer.getExecutable(),
				mCopyCommandBuffer.getExecutable(),
				mReleaseOwnershipBuffer.getExecutable(),
			};

			auto sem = semaphore();
			vk::SubmitInfo submit;
			submit
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores(&sem)
				.setCommandBufferCount(3)
				.setPCommandBuffers(buffers);
			context->transferQueue().submit(1, &submit, nullptr);

			// prepare command buffer aquiring ownership of resources on main queue
			mTakeOwnershipBuffer.beginRecording();
			mTakeOwnershipBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eVertexInput,
				{},
				0, nullptr,
				static_cast<uint32_t>(mBufferOwnershipBarriers.size()),
				mBufferOwnershipBarriers.data(),
				static_cast<uint32_t>(mImageOwnershipBarriers.size()),
				mImageOwnershipBarriers.data()
			);
			mTakeOwnershipBuffer.endRecording();

			// clear all arrays
			mBufferOwnershipBarriers.clear();
			mImageOwnershipBarriers.clear();
		
			mBufferInitialBarriers.clear();
			mImageInitialBarriers.clear();

			mTasksAdded = false;
			mResourcesAvailable = true;

			// prepare command buffer for recording new transfer commands
			mCopyCommandBuffer.beginRecording();
		}
	}

	void ResourceUploader::releaseResources() {
		mStagingBuffers[mCurrentIndex].clear();
	}

	ResourceUploader * resourceUploader;

}
