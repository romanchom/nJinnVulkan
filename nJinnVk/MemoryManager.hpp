#pragma once

#include <cstdint>
#include <map>
#include <vulkan.hpp>

#include "BucketAllocator.hpp"

namespace nJinn {
	class MemoryManager {
	private:
		static std::multimap<size_t, BucketAllocator> sBuckets;
		static void freeBucketAllocator(BucketAllocator * bucketAlloc);

		friend class BucketAllocator;
	public:
		enum {
			MiB = 1024 * 1024,
			bigObjectSize = 2 * MiB,
			maxBucketAllocatorSize = 4 * MiB,
		};	
		static std::pair<Allocator *, size_t> alloc(size_t size);
	};
}
