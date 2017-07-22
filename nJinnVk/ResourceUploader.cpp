#include "stdafx.hpp"
#include "ResourceUploader.hpp"

#include "Context.hpp"

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
		mCurrentIndex(0),
		mTasksAdded(false)
	{
		mCommandBuffer.beginRecording();
	}

	ResourceUploader::~ResourceUploader() {}

	void ResourceUploader::uploadBuffer(StagingBuffer source, vk::Buffer destination) {
		// TODO add synchronization if necessary
		vk::BufferCopy region;
		region
			.setSrcOffset(0)
			.setDstOffset(0)
			.setSize(source.size());
		mCommandBuffer->copyBuffer(source.buffer(), destination, 1, &region);

		mStagingBuffers[mCurrentIndex].emplace_back(std::move(source));
		mTasksAdded = true;
	}

	void ResourceUploader::execute()
	{
		auto sem = mTransfersCompleteSemaphore.get();

		vk::SubmitInfo submit;
		submit
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&sem);

		if (mTasksAdded) {
			mCommandBuffer.endRecording();

			vk::CommandBuffer buffer = mCommandBuffer.get();

			submit
				.setCommandBufferCount(1)
				.setPCommandBuffers(&buffer);

			++mCurrentIndex %= 2;

			mStagingBuffers[mCurrentIndex].clear();
			mTasksAdded = false;

			mCommandBuffer.beginRecording();
		}

		context->mainQueue().submit(1, &submit, nullptr);
	}

	ResourceUploader * resourceUploader;

}
