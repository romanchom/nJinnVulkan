#include "stdafx.hpp"
#include "SegregatedAllocator.hpp"

#include <stdexcept>

#include <intrin.h>

#include "Context.hpp"
#include "Debug.hpp"

namespace nJinn {
	using namespace detail;

	void SegregatedAllocator::allocateChunk() {
		// allocate chunk
		mChunks.emplace_front();
		auto chunk = mChunks.begin();
		auto ret = context->dev().allocateMemory(&mAllocateInfo, nullptr, &chunk->memory);
		if (vk::Result::eSuccess != ret) {
			throw std::runtime_error("Device memory allocation failed");
		}
		if (mShouldMap) {
			chunk->mapping = reinterpret_cast<uint8_t*>(context->dev()
				.mapMemory(chunk->memory, 0, mChunkSize));
		}

		// create one large block
		auto block = createEmptyBlock(chunk, chunk->blocks.begin());
		block->size = mChunkSize;
		block->offset = 0;

		createFreeNode(block);
	}

	BlockHandle SegregatedAllocator::createEmptyBlock(ChunkHandle chunk, BlockHandle position) {
		auto block = chunk->blocks.emplace(position);
		block->chunk = chunk;
		return block;
	}

	void SegregatedAllocator::createFreeNode(BlockHandle block) {
		auto & freelist = mFreeLists[blockSizeClass(block->size)];
		freelist.emplace_front(block);
		auto free = freelist.begin();
		block->free = free;
	}

	uint64_t SegregatedAllocator::allocationSizeClass(vk::DeviceSize size) {
		// get size class of sufficient size

		// get size twice as large
		// subtract one
		// get the next power of two greater than this number
		// example 
		// log2(13 * 2 - 1) = log2(25) = 4 (size class 2^4 = 16, ok large enough)
		// log2(1 * 2 - 1) = log2(1) = 0 (size class 2^0 = 1, ok exact fit)
		// log2(4 * 2 - 1) = log2(7) = 2 (size class 2^2 = 4, ok exact fit)
		return static_cast<uint32_t>(blockSizeClass((size << 1) - 1));
	}

	uint64_t SegregatedAllocator::blockSizeClass(vk::DeviceSize size) {
		unsigned long index;
		_BitScanReverse64(&index, size);
		return static_cast<uint64_t>(index);
	}

	SegregatedAllocator::SegregatedAllocator(vk::DeviceSize size, vk::DeviceSize alignment, uint32_t memoryType, bool shouldMap) :
		mChunkSize(size),
		mAligner(alignment),
		mListCount(blockSizeClass(size) + 1),
		mShouldMap(shouldMap)
	{
		mFreeLists = std::make_unique<FreeList[]>(mListCount);

		mAllocateInfo
			.setAllocationSize(mChunkSize)
			.setMemoryTypeIndex(memoryType);

		allocateChunk();

	}

	SegregatedAllocator::~SegregatedAllocator() {
		for (auto & chunk : mChunks) {
#ifdef _DEBUG
			auto & blocks = chunk.blocks;
			auto second = blocks.begin();
			++second;
			if (blocks.end() != second) {
				debug->error("Memory chunk not free when released");
			}
#endif
			if (mShouldMap) {
				context->dev().unmapMemory(chunk.memory);
			}
			context->dev().freeMemory(chunk.memory);
		}
	}

	SegregatedAllocator::Allocation SegregatedAllocator::alloc(vk::DeviceSize size) {
		Allocation ret;

		// align
		size = mAligner(size);

		uint32_t index = allocationSizeClass(size);
		// TODO provide best fit method which searches blocks of size class one smaller
		bool success = false;
		// iterate all free list of required size
		while (index < mListCount) {
			auto & list = mFreeLists[index];
			auto freeBlock = list.begin();
			if (list.end() != freeBlock) {
				// if a block of required size exists
				auto block = *freeBlock;
				// remove from old free list
				list.erase(freeBlock);
				ret.mBlock = block;
				// mark as busy by assigning new list end iterator to free
				block->free = mFreeLists[blockSizeClass(size)].end();
				success = true;
				// get the remaining size
				uint32_t sizeLeft = static_cast<uint32_t>(block->size - size);
				if (0 != sizeLeft) {
					// insert a split right after out block
					auto next = block;
					++next;

					// block needs to be split
					auto split = createEmptyBlock(block->chunk, next);
					split->size = sizeLeft;
					split->offset = block->offset + size;

					createFreeNode(split);

					// update block size
					block->size = size;
				}
				break;
			}
			++index;
		}

		if (!success) {
			if (size > mChunkSize) {
				throw std::bad_alloc();
			}
			else {
				allocateChunk();
				ret = alloc(size);
			}
		}
		return ret;
	}

	void SegregatedAllocator::free(Allocation & allocation) {
		auto block = allocation.mBlock;
		auto & chunk = block->chunk->blocks;
		auto next = block;
		++next;
		// coalesce with next
		if (chunk.end() != next) {
			auto & nextFreeList = mFreeLists[blockSizeClass(next->size)];
			if (nextFreeList.end() != next->free) {
				block->size += next->size;
				nextFreeList.erase(next->free);
				chunk.erase(next);
			}
		}
		if (chunk.begin() != block) {
			auto prev = block;
			--prev;
			auto & prevFreeList = mFreeLists[blockSizeClass(prev->size)];
			if (prevFreeList.end() != prev->free) {
				block->size += prev->size;
				block->offset -= prev->size;
				prevFreeList.erase(prev->free);
				chunk.erase(prev);
			}
		}
		createFreeNode(block);
	}

	void SegregatedAllocator::validate() {
		for (auto && chunk : mChunks) {
			vk::DeviceSize prev = 0;
			for (auto && block : chunk.blocks) {
				if (block.offset != prev) {
					throw std::runtime_error("Device memory invalid");
				}
				prev += block.size;
			}
		}
	}
}
