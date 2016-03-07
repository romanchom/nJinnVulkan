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
	};

	class UniformBufferBase {
	private:
		static std::list<UniformAllocator> sAllocators;
	private:
		size_t mSize;
		size_t mCurrentOffset;
		UniformAllocator * mAllocator;
	public:
		UniformBufferBase(size_t size);
		~UniformBufferBase();

		void * acquirePointer();
		size_t offset() { return mCurrentOffset; }

		static void collect();
		static void update();
	};

	template<typename T>
	class UniformBuffer : public UniformBufferBase {
		UniformBuffer() :
			UniformBufferBase(sizeof(T))
		{}
		T * acquire() {
			return reinterpret_cast<T *>(acquirePointer());
		}
	};
}