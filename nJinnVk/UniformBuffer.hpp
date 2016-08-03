#pragma once

#include <list>
#include <vulkan.hpp>
#include "MemoryAllocation.hpp"

namespace nJinn {
	class UniformAllocator {
	private:
		vk::Buffer mBuffer;
		vk::DeviceMemory mMemory;
		size_t mTotalSpace;
		size_t mFreeSpace;
		size_t mCurrentOffset;
		char * mPointer;
		uint32_t mCycle;
	public:
		UniformAllocator(size_t uniformSize);
		~UniformAllocator();
		bool allocate(size_t size) {
			if (mFreeSpace < size) return false;
			mFreeSpace -= size;
			return true;
		}
		void free(size_t size) {
			mFreeSpace += size;
		}
		void update();
		std::pair<void *, size_t> acquire(size_t size);
		void writtenRange(vk::MappedMemoryRange & range);
		bool occupied();
		bool operator<(const UniformAllocator & that);
		vk::Buffer buffer() { return mBuffer; }
	};

	class UniformBuffer {
	private:
		static std::list<UniformAllocator> sAllocators;
	private:
		uint32_t mSize;
		uint32_t mCurrentOffset;
		UniformAllocator * mAllocator;
	public:
		~UniformBuffer();
		void initialize(uint32_t size);

		void * acquirePointer();
		template<typename T>
		T * acquire();

		uint32_t offset() { return mCurrentOffset; }
		void fillDescriptorInfo(vk::DescriptorBufferInfo & info);

		static void collect();
		static void update();
	};

	template<typename T>
	inline T * UniformBuffer::acquire() {
		return reinterpret_cast<T *>(acquirePointer());
	}
}