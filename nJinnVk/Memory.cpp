#include "stdafx.hpp"
#include "Memory.hpp"

#include "Context.hpp"

namespace nJinn {
	using namespace nJinn::literals;
	Memory::Memory() :
		mLocalAllocator(64_MiB, 256, context->bufferMemoryType(), false),
		mUploadAllocator(64_MiB, 256, context->uploadMemoryType(), true)
	{}

	Memory * memory = nullptr;
}

