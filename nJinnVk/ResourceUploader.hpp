#pragma once

#include <vector>

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



	/*namespace detail {
		class CopyTask {
		protected:
			std::unique_ptr<StagingBuffer> mSource;
		public:
			explicit CopyTask(std::unique_ptr<StagingBuffer> source) :
				mSource(std::move(source))
			{}
			virtual ~CopyTask() {};
			virtual void copy(vk::CommandBuffer cmdbuf) = 0;
		};

		class CopyTaskBufferToBuffer : public CopyTask {
		private:
			vk::Buffer mDestination;
		public:
			CopyTaskBufferToBuffer(std::unique_ptr<StagingBuffer> source, vk::Buffer destination) :
				CopyTask(std::move(source)),
				mDestination(destination)
			{}
			virtual void copy(vk::CommandBuffer cmdbuf) override;
		};
	}*/

	class ResourceUploader {
	private:
		CommandBuffer mCommandBuffer;

		std::vector<StagingBuffer> mStagingBuffers[2];
		size_t mCurrentIndex;
		Semaphore mTransfersCompleteSemaphore;
		bool mTasksAdded;
	public:
		ResourceUploader();
		~ResourceUploader();

		//void upload(const void * data, size_t size, vk::Buffer dst);
		void uploadBuffer(StagingBuffer source, vk::Buffer destination);
		void execute();
		vk::Semaphore semaphore() { return mTransfersCompleteSemaphore.get(); }
	};

	extern ResourceUploader * resourceUploader;
}