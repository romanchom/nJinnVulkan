#pragma once

#include <list>
#include <vulkan.hpp>
#include "Memory.hpp"

namespace nJinn {
	class UniformAllocator {
	private:
		vk::Buffer mBuffer;
		MemoryAllocation mMemory;
		uint32_t mTotalSpace;
		uint32_t mFreeSpace;
		uint32_t mCurrentOffset;
		uint8_t * mPointer;
		uint32_t mCycle;
	public:
		UniformAllocator(uint32_t uniformSize);
		~UniformAllocator();
		bool reserve(uint32_t size) {
			if (mFreeSpace < size) return false;
			mFreeSpace -= size;
			return true;
		}
		void release(uint32_t size) {
			mFreeSpace += size;
		}
		void update();
		uint32_t allocate(uint32_t size) noexcept;
		void * pointer(uint32_t offset) const noexcept {
			return mPointer + offset;
		}
		void flush();
		bool isFree() const noexcept;
		bool operator<(const UniformAllocator & that) const noexcept;
		vk::Buffer buffer() const noexcept { return mBuffer; }
	};

	class UniformManager {
	private:
		vk::DeviceSize mAllocatorSize;
		std::list<UniformAllocator> mAllocators;
	public:
		UniformManager(vk::DeviceSize allocatorSize);
		~UniformManager();
		UniformAllocator * reserve(uint32_t size);
		void collect() noexcept;
		void update();
	};

	extern UniformManager * uniformManager;

	class UniformBuffer {
	private:
	private:
		uint32_t mSize;
		uint32_t mCurrentOffset;
		UniformAllocator * mAllocator;
	public:
		~UniformBuffer();
		void initialize(uint32_t size);
		template<typename T>
		inline void initialize() {
			initialize(sizeof(T));
		}

		void advance() noexcept;
		template<typename T>
		T * acquire();

		uint32_t offset() const noexcept { return mCurrentOffset; }
		void fillDescriptorInfo(vk::DescriptorBufferInfo & info) const noexcept;
	};

	template<typename T>
	inline T * UniformBuffer::acquire() {
		return reinterpret_cast<T *>(mAllocator->pointer(mCurrentOffset));
	}
}