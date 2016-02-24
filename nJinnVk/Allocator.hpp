#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class Allocator {
	public:
		virtual ~Allocator() {};
		virtual size_t alloc() = 0;
		virtual void free(size_t offset) = 0;
		virtual vk::DeviceMemory memory() const = 0;
	};
}
