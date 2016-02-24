#pragma once

#include <cstdint>
#include <vulkan.hpp>

#include "Allocator.hpp"
#include "MemoryAllocation.hpp"

namespace nJinn {
	class BucketAllocator : public Allocator {
	private:
		size_t mBucketSize;
		uint64_t mBucketMask;
		uint64_t mBucketOccupied;
		MemoryAllocation mAllocation;
	public:
		BucketAllocator(size_t size);
		virtual ~BucketAllocator() override;
		virtual size_t alloc() override;
		virtual void free(size_t offset) override;
		virtual vk::DeviceMemory memory() const override;

		size_t bucketSize() { return mBucketSize; }
		bool freeBuckets() { return (~mBucketOccupied); }
	};
}
