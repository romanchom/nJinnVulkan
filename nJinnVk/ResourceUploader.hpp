#pragma once

#include <vector>
#include <mutex>

#include <vulkan.hpp>

#include "Memory.hpp"
#include "CommandBuffer.hpp"
#include "Semaphore.hpp"

namespace nJinn {
	class StagingBuffer {
	private:
		vk::Buffer mBuffer;
		MemoryAllocation mMemory;
	public:
		StagingBuffer() : mBuffer(nullptr) {}
		StagingBuffer(vk::DeviceSize size);
		StagingBuffer(StagingBuffer && orig) :
			mBuffer(orig.mBuffer),
			mMemory(orig.mMemory)
		{
			orig.mBuffer = nullptr;
		}
		StagingBuffer & operator=(StagingBuffer && orig) {
			mBuffer = orig.mBuffer;
			mMemory = orig.mMemory;
			orig.mBuffer = nullptr;
		}
		StagingBuffer(const StagingBuffer &) = delete;
		StagingBuffer & operator=(const StagingBuffer &) = delete;
		~StagingBuffer();
		vk::Buffer buffer() { return mBuffer; }
		vk::DeviceSize size() { return mMemory.size(); }
		void * pointer() { return mMemory.mapping(); }
	};

	class ResourceUploader {
	private:
		CommandBuffer mMainCommandBuffer;
		CommandBuffer mCopyCommandBuffer;
		CommandBuffer mReleaseOwnershipBuffer;
		CommandBuffer mTakeOwnershipBuffer;

		std::vector<StagingBuffer> mStagingBuffers[2];
		std::vector<vk::BufferMemoryBarrier> mBufferInitialBarriers;
		std::vector<vk::BufferMemoryBarrier> mBufferOwnershipBarriers;
		std::vector<vk::ImageMemoryBarrier> mImageInitialBarriers;
		std::vector<vk::ImageMemoryBarrier> mImageOwnershipBarriers;

		size_t mCurrentIndex;
		Semaphore mTransfersCompleteSemaphores[2];
		std::mutex mMutex;

		bool mTasksAdded;
		bool mResourcesAvailable;
	public:
		ResourceUploader();
		~ResourceUploader();
		void uploadBuffer(StagingBuffer source, vk::Buffer destination);
		void uploadImage(StagingBuffer source, class Image & destination);
		void execute();
		void releaseResources();
		vk::CommandBuffer takeOwnershipCommandBuffer() const noexcept { return mTakeOwnershipBuffer.getExecutable(); }
		bool resourcesAvailable() const noexcept { return mResourcesAvailable; }
		vk::Semaphore semaphore() { return mTransfersCompleteSemaphores[mCurrentIndex].get(); }
	};

	extern ResourceUploader * resourceUploader;
}