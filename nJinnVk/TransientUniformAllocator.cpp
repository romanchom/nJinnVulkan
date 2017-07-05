#include "stdafx.hpp"
#include "TransientUniformAllocator.h"

#include "Screen.hpp"

namespace nJinn {
	TransientUniformChunk::TransientUniformChunk(vk::DeviceSize size) :
		mSize(size),
		mOffset(0),
		mBytesFree(size),
		mBytesFreeReference(size)
	{
		for (int i = 0; i < Context::maxBufferedFrames; ++i) {
			mBytesAllocated[i] = mSize;
		}

		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		mBuffer = context->dev().createBuffer(bufferInfo);
		vk::MemoryRequirements memReq = context->dev().getBufferMemoryRequirements(mBuffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.setAllocationSize(memReq.size)
			.setMemoryTypeIndex(context->uploadMemoryType());

		mMemory = context->dev().allocateMemory(allocInfo);
		context->dev().bindBufferMemory(mBuffer, mMemory, 0);
		mMappedPointer = reinterpret_cast<char *>(context->dev().mapMemory(mMemory, 0, size, {}));
	}

	TransientUniformChunk::~TransientUniformChunk()
	{
		context->dev().destroyBuffer(mBuffer);
		context->dev().unmapMemory(mMemory);
		context->dev().freeMemory(mMemory);
	}

	bool TransientUniformChunk::isFree()
	{
		return (mBytesFree == mSize);
	}

	bool TransientUniformChunk::allocate(vk::DeviceSize size, TransientAllocation & ret)
	{
		if (mBytesFree < size) return false;
		if (mOffset + size > mSize) {
			mBytesFree -= mSize - mOffset;
			mOffset = 0;
			if (mBytesFree < size) return false;
		}
		ret.data = reinterpret_cast<void *>(mMappedPointer + mOffset);
		mOffset += size;
		ret.offset = mOffset;
		ret.descriptorSet = mDescriptorSet.get();
		return true;
	}

	void TransientUniformChunk::nextCycle()
	{
		// calculate number of bytes allocated in this cycle
		auto bytesAllocated = mBytesFreeReference - mBytesFree;
		if (!context->isUploadMemoryTypeCoherent()) {
			vk::MappedMemoryRange ranges[2];
			uint32_t count;
			if (bytesAllocated > mOffset) {
				// buffer wrapped around during allocation
				ranges[0]
					.setMemory(mMemory)
					.setOffset(mOffset + mSize - bytesAllocated)
					.setSize(bytesAllocated - mOffset);

				ranges[1]
					.setMemory(mMemory)
					.setOffset(0)
					.setSize(mOffset);

				count = 2;
			}else{
				ranges[0]
					.setMemory(mMemory)
					.setOffset(mOffset - bytesAllocated)
					.setSize(bytesAllocated);

				count = 1;
			}

			context->dev().flushMappedMemoryRanges(count, ranges);
		}

		auto oldestCycle = screen->maxQueuedFrames() - 1;
		auto bytesToFree = mBytesAllocated[oldestCycle];
		// shift history to the right
		for (int i = oldestCycle; i > 0; --i) {
			mBytesAllocated[i] = mBytesAllocated[i - 1];
		}
		mBytesAllocated[0] = bytesAllocated;
		// free no longer used memory
		mBytesFree = mBytesFree + bytesToFree;
		mBytesFreeReference = mBytesFree;
	}

	void TransientUniformAllocator::allocateChunk(vk::DeviceSize size)
	{
		mChunks.emplace_back(size);
		mCurrentChunk = &mChunks.back();
		mDescriptorAllocator.allocateDescriptorSet(mCurrentChunk->mDescriptorSet);
		mCurrentChunk->mDescriptorSet.write().uniformBuffer(mCurrentChunk->mBuffer, 0, size, 0);
	}

	TransientUniformAllocator::TransientUniformAllocator() :
		mDescriptorAllocator(16)
	{
		vk::DescriptorSetLayoutBinding binding;
		binding
			.setBinding(0)
			.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(1);

		mDescriptorAllocator.initialize(&binding, 1);

	}

	TransientUniformAllocator::~TransientUniformAllocator()
	{
	}

	TransientAllocation TransientUniformAllocator::allocate(vk::DeviceSize size)
	{
		TransientAllocation ret;
		if(!mCurrentChunk->allocate(size, ret)) {
			mChunks.emplace_back(mCurrentChunk->size() * 2);
			mCurrentChunk = &mChunks.back();
			mDescriptorAllocator.allocateDescriptorSet(mCurrentChunk->mDescriptorSet);
			mCurrentChunk->allocate(size, ret);
		}
		return ret;
	}

	void TransientUniformAllocator::nextCycle()
	{
		for (auto && chunk : mChunks) {
			chunk.nextCycle();
		}
		while (mChunks.size() > 1) {
			if (mChunks.front().isFree()) {
				mChunks.pop_front();
			}
			else {
				break;
			}
		}
	}

}