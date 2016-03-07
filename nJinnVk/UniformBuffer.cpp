#include "stdafx.hpp"
#include "UniformBuffer.hpp"

#include "Context.hpp"

namespace nJinn {
	UniformAllocator::UniformAllocator(size_t uniformSize) :
		mTotalSpace(uniformSize),
		mFreeSpace(uniformSize),
		mCurrentOffset(0),
		mCycle(0)
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.size(uniformSize * 2)
			.usage(vk::BufferUsageFlagBits::eUniformBuffer);

		dc(vk::createBuffer(Context::dev(), &bufferInfo, nullptr, &mBuffer));

		vk::MemoryRequirements memReq;
		vk::getBufferMemoryRequirements(Context::dev(), mBuffer, &memReq);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.allocationSize(memReq.size())
			.memoryTypeIndex(Context::uploadMemoryType());

		dc(vk::allocateMemory(Context::dev(), &allocInfo, nullptr, &mMemory));
		dc(vk::bindBufferMemory(Context::dev(), mBuffer, mMemory, 0));
		dc(vk::mapMemory(Context::dev(), mMemory, 0, uniformSize * 2, vk::MemoryMapFlags(), reinterpret_cast<void **>(&mPointer)));
	}

	UniformAllocator::~UniformAllocator()
	{
		vk::destroyBuffer(Context::dev(), mBuffer, nullptr);
		vk::unmapMemory(Context::dev(), mMemory);
		vk::freeMemory(Context::dev(), mMemory, nullptr);
	}

	void UniformAllocator::update()
	{
		++mCycle %= 2;
		mCurrentOffset = mTotalSpace * mCycle;
	}

	std::pair<void*, size_t> UniformAllocator::acquire(size_t size)
	{
		std::pair<void *, size_t> ret;
		ret.first = mPointer + mCurrentOffset;
		ret.second = mCurrentOffset;
		mCurrentOffset += size;
		return ret;
	}

	void UniformAllocator::writtenRange(vk::MappedMemoryRange & range)
	{
		range
			.memory(mMemory)
			.offset(mCycle * mTotalSpace)
			.size(mCurrentOffset);
	}

	bool UniformAllocator::occupied()
	{
		return mFreeSpace == mTotalSpace;
	}

	bool UniformAllocator::operator<(const UniformAllocator & that)
	{
		return mFreeSpace > that.mFreeSpace;
	}

	bool isFree(UniformAllocator & alloc) {
		return !alloc.occupied();
	}

	void UniformBufferBase::collect()
	{
		sAllocators.remove_if(isFree);
		sAllocators.sort();
	}

	void UniformBufferBase::update()
	{
		if (!Context::isUploadMemoryTypeCoherent()) {
			vk::MappedMemoryRange * ranges = new vk::MappedMemoryRange[sAllocators.size()];
			int i = 0;
			for (UniformAllocator & alloc : sAllocators) {
				alloc.writtenRange(ranges[i++]);
			}
			dc(vk::flushMappedMemoryRanges(Context::dev(), i, ranges));
		}
		for (UniformAllocator & alloc : sAllocators) {
			alloc.update();
		}
	}

	UniformBufferBase::UniformBufferBase(size_t size) :
		mSize(size)
	{
		int tryCount = sAllocators.size();
		while (tryCount > 0) {
			auto it = sAllocators.begin();
			if (it->allocate(size)) {
				mAllocator = &*it;
				return;
			} else {
				// move full allocator to the back
				sAllocators.splice(sAllocators.end(), sAllocators, it);
				--tryCount;
			}
		}
		sAllocators.emplace_front(4 * 1024 * 1024);
		mAllocator = &sAllocators.front();
		mAllocator->allocate(size);
	}

	UniformBufferBase::~UniformBufferBase()
	{
		mAllocator->free(mSize);
	}

	void * UniformBufferBase::acquirePointer()
	{
		auto v = mAllocator->acquire(mSize);
		mCurrentOffset = v.second;
		return v.first;
	}
}
