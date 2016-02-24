#pragma once

#include <vulkan.hpp>

#include "Allocator.hpp"

namespace nJinn {
	class MemoryAllocation {
	private:
		class Allocator * mAllocator;
		size_t mOffset;
	public:
		MemoryAllocation();
		~MemoryAllocation();
		void allocate(size_t size);
		vk::DeviceMemory deviceMemory() const { return mAllocator->memory(); }
		operator vk::DeviceMemory() const { return deviceMemory(); }
		size_t offset() const { return mOffset; }
	};
}
