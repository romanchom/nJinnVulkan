#pragma once

#include <deque>
#include <vulkan.hpp>
#include "DescriptorSet.hpp"
#include "DescriptorAllocator.hpp"
#include "Context.hpp"

namespace nJinn {
	struct TransientAllocation {
		vk::DescriptorSet descriptorSet;
		void * data;
		uint32_t offset;
	};

	class TransientUniformChunk {
	private:
		DescriptorSet mDescriptorSet;
		// Temporary own memory allocation
		// TODO obtain memory from separate preallocated device memory
		vk::DeviceMemory mMemory;
		vk::Buffer mBuffer;

		vk::DeviceSize mSize;
		vk::DeviceSize mBytesAllocated[Context::maxBufferedFrames];
		vk::DeviceSize mOffset;
		vk::DeviceSize mBytesFree;
		vk::DeviceSize mBytesFreeReference;

		char * mMappedPointer;
	public:
		TransientUniformChunk(vk::DeviceSize size);
		~TransientUniformChunk();

		bool isFree();
		bool allocate(vk::DeviceSize size, TransientAllocation & ret);
		vk::DeviceSize size() const noexcept { return mSize; }

		void nextCycle();

		friend class TransientUniformAllocator;
	};

	class TransientUniformAllocator
	{
	private:
		TransientUniformChunk * mCurrentChunk;
		DescriptorAllocator mDescriptorAllocator;
		std::deque<TransientUniformChunk> mChunks;
		void allocateChunk(vk::DeviceSize size);
	public:
		TransientUniformAllocator();
		~TransientUniformAllocator();

		TransientAllocation allocate(vk::DeviceSize size);
		void nextCycle();
	};

}
