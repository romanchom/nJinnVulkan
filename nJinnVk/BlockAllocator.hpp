#pragma once

#include <vulkan.hpp>

#include "Allocator.hpp"

namespace nJinn {
	class BlockAllocator : public Allocator {
	private:
		vk::DeviceMemory mDeviceMemory;
		bool mOccupied;
	public:
		BlockAllocator(size_t size);
		virtual ~BlockAllocator() override;
		virtual size_t alloc() override;
		virtual void free(size_t offset) override;
		virtual vk::DeviceMemory memory() const override;
	};
}
