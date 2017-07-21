#pragma once

#include <cstdint>
#include <map>
#include <vulkan.hpp>

#include "SegregatedAllocator.hpp"

namespace nJinn {
	namespace literals {
		constexpr vk::DeviceSize operator""_KiB(unsigned long long kibibytes) {
			return static_cast<vk::DeviceSize>(kibibytes << 10);
		}

		constexpr vk::DeviceSize operator""_MiB(unsigned long long mebibytes) {
			return static_cast<vk::DeviceSize>(mebibytes << 20);
		}

		constexpr vk::DeviceSize operator""_GiB(unsigned long long gibibytes) {
			return static_cast<vk::DeviceSize>(gibibytes << 30);
		}
	}

	class Memory {
	private:
		SegregatedAllocator mLocalAllocator;
		SegregatedAllocator mUploadAllocator;
	public:
		Memory();
		SegregatedAllocator & local() {
			return mLocalAllocator;
		}
		SegregatedAllocator & upload() {
			return mUploadAllocator;
		}
	};

	using MemoryAllocation = SegregatedAllocator::Allocation;

	extern Memory * memory;
}
