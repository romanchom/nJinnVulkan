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
		auto memReq = context->dev().getBufferMemoryRequirements(mBuffer);

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

	uint32_t UniformAllocator::allocate(uint32_t size) noexcept {
		auto ret = mCurrentOffset;
		mCurrentOffset += size;
		return ret;
	}

	void UniformAllocator::flush() {
		auto start = static_cast<vk::DeviceSize>(mCycle * mTotalSpace);
		auto size = static_cast<vk::DeviceSize>(mCurrentOffset) - start;
		mMemory.flush(start, size);
	}

	bool UniformAllocator::isFree() const noexcept {
		return mFreeSpace == mTotalSpace;
	}

	bool UniformAllocator::operator<(const UniformAllocator & that) const  noexcept {
		return mFreeSpace > that.mFreeSpace;
	}

	UniformManager::UniformManager(vk::DeviceSize allocatorSize) :
		mAllocatorSize(allocatorSize)
	{
		mAllocators.emplace_front(static_cast<uint32_t>(mAllocatorSize));
	}
	
	UniformManager::~UniformManager() {}
	
	UniformAllocator * UniformManager::reserve(uint32_t size) {
		int tryCount = (int) mAllocators.size();
		while (tryCount > 0) {
			auto it = mAllocators.begin();
			if (it->reserve(size)) {
				return &*it;
			}
			else {
				// move full allocator to the back
				mAllocators.splice(mAllocators.end(), mAllocators, it);
				--tryCount;
			}
		}

		mAllocators.emplace_front(static_cast<uint32_t>(mAllocatorSize));
		auto & ret = mAllocators.front();
		ret.reserve(size);
		return &ret;
	}

	void UniformManager::collect() noexcept {
		for (auto it = mAllocators.begin(); mAllocators.end() != it;) {
			if (it->isFree()) {
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

	UniformBuffer::~UniformBuffer() {
		mAllocator->release(mSize);
	}

	void UniformBuffer::initialize(uint32_t size) {
		mSize = static_cast<uint32_t>(context->alignUniform(size));
		mAllocator = uniformManager->reserve(mSize);
	}

	void UniformBuffer::advance() noexcept {
		mCurrentOffset = mAllocator->allocate(mSize);
	}

	void UniformBuffer::fillDescriptorInfo(vk::DescriptorBufferInfo & info) const noexcept {
		info
			.setBuffer(mAllocator->buffer())
			.setOffset(0)
			.setRange(mSize);
	}
}
