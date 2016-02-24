#include "stdafx.hpp"
#include "MemoryAllocation.hpp"

#include "MemoryManager.hpp"

namespace nJinn {
	MemoryAllocation::MemoryAllocation() :
		mAllocator(nullptr),
		mOffset(-1)
	{}

	void MemoryAllocation::allocate(size_t size) {
		auto alloc = MemoryManager::alloc(size);
		mAllocator = alloc.first;
		mOffset = alloc.second;
	}

	MemoryAllocation::~MemoryAllocation() {
		if(mAllocator) mAllocator->free(mOffset);
	}
}
