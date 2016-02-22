#pragma once

#include <cstdint>

namespace vbm {
	struct Header {
		enum { magickNumberValue = 0x13243546 };
		const uint32_t magickNumber;
		uint32_t indiciesCount;
		uint32_t indexTypeSize;
		uint32_t vertexCount;
		uint32_t vetexStreamCount;
		uint32_t totalDataSize;
		Header() : magickNumber(magickNumberValue), indiciesCount(0), indexTypeSize(0), vertexCount(0), totalDataSize(0) {}
	};

	struct VertexStream {
		uint32_t vertexAttributeCount;
		uint32_t offsetFromBufferBegin;
		uint32_t stride;
	};

	struct VertexAttribute {
		uint32_t type;
		uint32_t componentCount;
		uint32_t offset;
		char name[16];
	};
}
