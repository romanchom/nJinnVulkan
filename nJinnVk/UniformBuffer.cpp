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
			.setSize(uniformSize * 2)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);
		
		mBuffer = context->dev().createBuffer(bufferInfo);
		vk::MemoryRequirements memReq = context->dev().getBufferMemoryRequirements(mBuffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.setAllocationSize(memReq.size)
			.setMemoryTypeIndex(context->uploadMemoryType());

		mMemory = context->dev().allocateMemory(allocInfo);
		context->dev().bindBufferMemory(mBuffer, mMemory, 0);
		mPointer = (char *) context->dev().mapMemory(mMemory, 0, uniformSize * 2, vk::MemoryMapFlags());
	}

	UniformAllocator::~UniformAllocator()
	{
		context->dev().destroyBuffer(mBuffer);
		context->dev().unmapMemory(mMemory);
		context->dev().freeMemory(mMemory);
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
			.setMemory(mMemory)
			.setOffset(mCycle * mTotalSpace)
			.setSize(mCurrentOffset);
	}

	bool UniformAllocator::occupied()
	{
		return mFreeSpace != mTotalSpace;
	}

	bool UniformAllocator::operator<(const UniformAllocator & that)
	{
		return mFreeSpace > that.mFreeSpace;
	}

	bool isFree(UniformAllocator & alloc) {
		return !alloc.occupied();
	}

	void UniformBuffer::collect()
	{
		sAllocators.remove_if(isFree);
		sAllocators.sort();
	}

	void UniformBuffer::update()
	{
		if (!context->isUploadMemoryTypeCoherent()) {
			vk::MappedMemoryRange * ranges = new vk::MappedMemoryRange[sAllocators.size()];
			int i = 0;
			for (UniformAllocator & alloc : sAllocators) {
				alloc.writtenRange(ranges[i++]);
			}
			context->dev().flushMappedMemoryRanges(i, ranges);
		}
		for (UniformAllocator & alloc : sAllocators) {
			alloc.update();
		}
	}

	std::list<UniformAllocator> UniformBuffer::sAllocators;

	UniformBuffer::~UniformBuffer()
	{
		mAllocator->free(mSize);
	}

	void UniformBuffer::initialize(uint32_t size)
	{
		mSize = size;
		int tryCount = (int) sAllocators.size();
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

	void * UniformBuffer::acquirePointer()
	{
		auto v = mAllocator->acquire(mSize);
		mCurrentOffset = v.second;
		return v.first;
	}

	void UniformBuffer::fillDescriptorInfo(vk::DescriptorBufferInfo & info)
	{
		info
			.setBuffer(mAllocator->buffer())
			.setOffset(0)
			.setRange(mSize);
	}
}
