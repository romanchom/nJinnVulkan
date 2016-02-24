#include "stdafx.hpp"
#include "BlockAllocator.hpp"

#include "Context.hpp"

namespace nJinn {
	BlockAllocator::BlockAllocator(size_t size) :
		mOccupied(false)
	{
		vk::MemoryAllocateInfo info;
		info
			.allocationSize(size)
			.memoryTypeIndex(Context::bufferMemoryType());
		vk::allocateMemory(Context::dev(), &info, nullptr, &mDeviceMemory);
	}

	BlockAllocator::~BlockAllocator()
	{
		vk::freeMemory(Context::dev(), mDeviceMemory, nullptr);
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
