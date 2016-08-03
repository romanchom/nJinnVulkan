#include "stdafx.hpp"
#include "MemoryManager.hpp"

#include "BlockAllocator.hpp"

namespace nJinn {
	std::multimap<size_t, BucketAllocator> MemoryManager::sBuckets;

	void MemoryManager::freeBucketAllocator(BucketAllocator * bucketAlloc)
	{
		auto range = sBuckets.equal_range(bucketAlloc->bucketSize());

		for (auto it = range.first; it != range.second; ++it) {
			if (&it->second == bucketAlloc) {
				sBuckets.erase(it);
				return;
			}
		}
	}

	std::pair<Allocator *, size_t> MemoryManager::alloc(size_t size)
	{
		std::pair<Allocator *, size_t> ret;
		if (size > bigObjectSize) {
			BlockAllocator * allocator = new BlockAllocator(size);
			ret.first = allocator;
			ret.second = allocator->alloc();
			// no need for delete, it will self destruct after it is freed
		} else {
			// get nearest power of 2 larger than size
			uint64_t pos;
#ifdef _WIN32
			pos = __lzcnt64(size - 1);
#else
			pos = __builtin_clzll(~size);
#endif
			pos = 64ull - pos;
			size = 1ull << pos;

			auto range = sBuckets.equal_range(size);

			bool allocationSuccessful = false;
			for (auto it = range.first; it != range.second; ++it) {
				// try to find a free bucket of desired size
				if (it->second.freeBuckets()) {
					ret.first = &it->second;
					ret.second = it->second.alloc();
					allocationSuccessful = true;
					break;
				}
			}

			if (!allocationSuccessful) {
				// allocate new buckets
				auto bucketAlloc = sBuckets.emplace(size, size);
				ret.first = &bucketAlloc->second;
				ret.second = bucketAlloc->second.alloc();
			}
		}
		return ret;
	}
}

