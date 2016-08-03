#include "stdafx.hpp"
#include "BlockAllocator.hpp"

#include "Context.hpp"

namespace nJinn {
	BlockAllocator::BlockAllocator(size_t size) :
		mOccupied(false)
	{
		vk::MemoryAllocateInfo info;
		info
			.setAllocationSize(size)
			.setMemoryTypeIndex(context->bufferMemoryType());
		mDeviceMemory = context->dev().allocateMemory(info);
	}

	BlockAllocator::~BlockAllocator()
	{
		context->dev().freeMemory(mDeviceMemory);
	}

	size_t BlockAllocator::alloc()
	{
		mOccupied = true;
		return 0;
	}

	void BlockAllocator::free(size_t offset)
	{
		assert(0 == offset);
		mOccupied = false;
		delete this;
	}

	vk::DeviceMemory BlockAllocator::memory() const
	{
		return mDeviceMemory;
	}
}
