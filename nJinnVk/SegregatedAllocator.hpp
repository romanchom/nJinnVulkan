#pragma once

#include <list>
#include <forward_list>
#include <vulkan.hpp>
#include <memory>
#include <boost/pool/pool_alloc.hpp>

#include "Math.hpp"

namespace nJinn {
	namespace detail {
		template<typename T>
		using MemoryAllocator = boost::pool_allocator<T>;
		template<typename T>
		using List = std::list<T, MemoryAllocator<T>>;

		struct Chunk;
		struct Block;

		using BlockList = List<Block>;
		using BlockHandle = BlockList::iterator;

		using FreeList = List<BlockHandle>;
		using FreeHandle = FreeList::iterator;

		using ChunkList = List<Chunk>;
		using ChunkHandle = ChunkList::iterator;

		struct Chunk {
			vk::DeviceMemory memory;
			BlockList blocks;
			uint8_t * mapping;
		};

		struct Block {
			vk::DeviceSize offset;
			vk::DeviceSize size;
			FreeHandle free;
			ChunkHandle chunk;
		};
	}

	class SegregatedAllocator {
	private:
		vk::DeviceSize mChunkSize;
		Aligner<vk::DeviceSize> mAligner;
		uint32_t mListCount;
		bool mShouldMap;
		vk::MemoryAllocateInfo mAllocateInfo;
		std::unique_ptr<detail::FreeList[]> mFreeLists;
		detail::ChunkList mChunks;

		void allocateChunk();
		detail::BlockHandle createEmptyBlock(detail::ChunkHandle chunk, detail::BlockHandle position);
		void createFreeNode(detail::BlockHandle block);

		static uint64_t allocationSizeClass(vk::DeviceSize size);
		static uint64_t blockSizeClass(vk::DeviceSize size);
	public:
		class Allocation {
		private:
			detail::BlockHandle mBlock;
		public:
			vk::DeviceMemory memory() { return mBlock->chunk->memory; }
			vk::DeviceSize offset() { return mBlock->offset; }
			vk::DeviceSize size() { return mBlock->size; }
			uint8_t * mapping() { return mBlock->chunk->mapping + offset(); }
			friend SegregatedAllocator;
		};
		SegregatedAllocator(vk::DeviceSize size, vk::DeviceSize alignment, uint32_t memoryType, bool shouldMap);
		~SegregatedAllocator();
		Allocation alloc(vk::DeviceSize size);
		void free(Allocation & allocation);
		void validate();
	};
}