#include "stdafx.hpp"
#include "UniformBuffer.hpp"

#include "Context.hpp"

namespace nJinn {
	UniformAllocator::UniformAllocator(uint32_t uniformSize) :
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

		mMemory = memory->upload().alloc(memReq.size);
		context->dev().bindBufferMemory(mBuffer, mMemory.memory(), mMemory.offset());
		mPointer = mMemory.mapping();
	}

	UniformAllocator::~UniformAllocator() {
		context->dev().destroyBuffer(mBuffer);
		memory->upload().free(mMemory);
	}

	void UniformAllocator::update() {
		// TODO where is this "2" comming from??
		++mCycle %= 2;
		mCurrentOffset = mTotalSpace * mCycle;
	}

	uint32_t UniformAllocator::obtain(uint32_t size) noexcept {
		uint32_t ret = mCurrentOffset;
		mCurrentOffset += size;
		return ret;
	}

	void UniformAllocator::writtenRange(vk::MappedMemoryRange & range)
	{
		/*range
			.setMemory(mMemory)
			.setOffset(mCycle * mTotalSpace)
			.setSize(mCurrentOffset);*/
	}

	bool UniformAllocator::free() const noexcept {
		return mFreeSpace == mTotalSpace;
	}

	bool UniformAllocator::operator<(const UniformAllocator & that) const  noexcept {
		return mFreeSpace > that.mFreeSpace;
	}

	UniformManager::UniformManager(vk::DeviceSize allocatorSize) :
		mAllocatorSize(allocatorSize)
	{
		mAllocators.emplace_front(mAllocatorSize);
	}
	
	UniformManager::~UniformManager() {}
	
	UniformAllocator * UniformManager::allocate(uint32_t size) {
		UniformAllocator * ret = nullptr;
		int tryCount = (int) mAllocators.size();
		while (tryCount > 0) {
			auto it = mAllocators.begin();
			if (it->allocate(size)) {
				return &*it;
			}
			else {
				// move full allocator to the back
				mAllocators.splice(mAllocators.end(), mAllocators, it);
				--tryCount;
			}
		}

		mAllocators.emplace_front(mAllocatorSize);
		return &mAllocators.front();
	}

	void UniformManager::collect() noexcept {
		for (auto it = mAllocators.begin(); mAllocators.end() != it;) {
			if (it->free()) {
				auto copy = it;
				++it;
				mAllocators.erase(copy);
			}
		}
	}

	void UniformManager::update() {
		for (auto && alloc : mAllocators) {
			alloc.update();
		}
	}

	UniformManager * uniformManager = nullptr;

	
	/*void UniformBuffer::update()
	{
		if (!context->isUploadMemoryTypeCoherent()) {
			// TODO fix this allocation and this fucking memory leak
			/*vk::MappedMemoryRange * ranges = new vk::MappedMemoryRange[sAllocators.size()];
			int i = 0;
			for (UniformAllocator & alloc : sAllocators) {
				alloc.writtenRange(ranges[i++]);
			}
			context->dev().flushMappedMemoryRanges(i, ranges);
		}
		for (UniformAllocator & alloc : sAllocators) {
			alloc.update();
		}
	}*/


	UniformBuffer::~UniformBuffer() {
		mAllocator->free(mSize);
	}

	void UniformBuffer::initialize(uint32_t size) {
		mSize = static_cast<uint32_t>(context->alignUniform(size));
		mAllocator = uniformManager->allocate(mSize);
		mAllocator->allocate(mSize);
	}

	void UniformBuffer::advance() noexcept {
		mCurrentOffset = mAllocator->obtain(mSize);
	}

	void UniformBuffer::fillDescriptorInfo(vk::DescriptorBufferInfo & info) const noexcept {
		info
			.setBuffer(mAllocator->buffer())
			.setOffset(0)
			.setRange(mSize);
	}
}
