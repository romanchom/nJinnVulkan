#include "stdafx.hpp"
#include "BucketAllocator.hpp"

#include <algorithm>

#include "MemoryManager.hpp"

namespace nJinn {
	BucketAllocator::BucketAllocator(size_t size) :
		mBucketSize(size)
	{
		size_t bucketCount = std::min(size * 64, (size_t)MemoryManager::maxBucketAllocatorSize) / size;
		mBucketMask = bucketCount >= 64 ? -1 : (static_cast<uint64_t>(1) << bucketCount) - 1;
		// treating nonexistent buckets as occupied simplifies math
		mBucketOccupied = ~mBucketMask;
		mAllocation.allocate(size * bucketCount);
	}

	BucketAllocator::~BucketAllocator()
	{
		assert((mBucketOccupied & mBucketMask) == 0);
	}

	size_t BucketAllocator::alloc()
	{
		// find first zero in bit map
		size_t pos;
#ifdef _WIN32
		pos = __lzcnt64(~mBucketOccupied);
#else
		pos = __builtin_clzll(~mBucketOccupied);
#endif
		pos = 63 - pos;
		assert(pos != -1);
		
		mBucketOccupied |= (static_cast<uint64_t>(1) << pos);

		return pos * mBucketSize;
	}

	void BucketAllocator::free(size_t offset)
	{
		size_t pos = offset / mBucketSize;
		assert(pos <= 63);
		mBucketOccupied &= ~(static_cast<uint64_t>(1) << pos);
		if (0 == (mBucketOccupied & mBucketMask)) {
			MemoryManager::freeBucketAllocator(this);
		}
	}

	vk::DeviceMemory BucketAllocator::memory() const
	{
		return mAllocation.deviceMemory();
	}
}
