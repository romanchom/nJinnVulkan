#include "stdafx.hpp"
#include "UniformBuffer.hpp"

#include "Context.hpp"
#include "Math.hpp"
#include "Debug.hpp"

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
		mPointer = (char *)context->dev().mapMemory(mMemory, 0, uniformSize * 2, {});
		//																	   ^
		//																	   |
		// TODO															or this "2"
	}

	UniformAllocator::~UniformAllocator()
	{
		context->dev().destroyBuffer(mBuffer);
		context->dev().unmapMemory(mMemory);
		context->dev().freeMemory(mMemory);
	}

	void UniformAllocator::update()
	{
		// TODO where is this "2" comming from??
		++mCycle %= 2;
		mCurrentOffset = mTotalSpace * mCycle;
	}

	std::pair<void*, size_t> UniformAllocator::acquire(size_t size)
	{
		std::pair<void *, size_t> ret;
		ret.first = mPointer + mCurrentOffset;
		ret.second = mCurrentOffset;
		// TODO make separate class for uniform buffer management and integrate it with startup system
		mCurrentOffset += context->alignUniform(size);
		return ret;
	}

	void UniformAllocator::writtenRange(vk::MappedMemoryRange & range)
	{
		range
			.setMemory(mMemory)
			.setOffset(mCycle * mTotalSpace)
			.setSize(mCurrentOffset);
	}

	bool UniformAllocator::occupied() const
	{
		return mFreeSpace != mTotalSpace;
	}

	bool UniformAllocator::operator<(const UniformAllocator & that)
	{
		return mFreeSpace > that.mFreeSpace;
	}

	void UniformBuffer::collect()
	{
		sAllocators.remove_if([](const UniformAllocator & alloc) { return !alloc.occupied(); });
		sAllocators.sort();
	}

	void UniformBuffer::update()
	{
		if (!context->isUploadMemoryTypeCoherent()) {
			// TODO fix this allocation and this fucking memory leak
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
		debug->log("Free  ", mSize, "\n");
		mAllocator->free(mSize);
	}

	void UniformBuffer::initialize(uint32_t size)
	{
		debug->log("Alloc ", size, "\n");
		mSize = context->alignUniform(size);
		int tryCount = (int) sAllocators.size();
		while (tryCount > 0) {
			auto it = sAllocators.begin();
			if (it->allocate(mSize)) {
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
		mAllocator->allocate(mSize);
	}

	void * UniformBuffer::acquirePointer()
	{
		auto v = mAllocator->acquire(mSize);
		mCurrentOffset = (uint32_t) v.second;
		return v.first;
	}

	void UniformBuffer::fillDescriptorInfo(vk::DescriptorBufferInfo & info) const
	{
		info
			.setBuffer(mAllocator->buffer())
			.setOffset(0)
			.setRange(mSize);
	}
}
