#pragma once

#include <string>

#include <boost/iostreams/device/mapped_file.hpp>

#include "AttributeType.hpp"
#include "VBM.hpp"

namespace meshLoader {
	class MeshData {
	public:
		MeshData(const std::string & fileName);
		~MeshData();

		const char * data() const { return dataStart; }
		const size_t totalDataSize() { return header->totalDataSize; }

		uint32_t vertexStreamCount() const { return header->vetexStreamCount; }
		const vbm::VertexStream & vertexStream(size_t index) const { return *vertexStreams[index]; }
		
		uint32_t attributeCount() const { return attributeCount_; }
		const vbm::VertexAttribute & vertexAttribute(size_t stream, size_t attr) const { return vertexAttributes[stream][attr]; }

		uint32_t indexCount() const { return header->indiciesCount; }
		uint32_t vertexCont() const { return header->vertexCount; }
		uint32_t indexSize() const { return header->indexTypeSize; }
	private:
		boost::iostreams::mapped_file_source file;
		const vbm::Header * header;
		const vbm::VertexStream ** vertexStreams;
		const vbm::VertexAttribute ** vertexAttributes;
		uint32_t attributeCount_;
		const char * dataStart;

		MeshData(const MeshData &) = delete;
		MeshData & operator=(const MeshData &) = delete;
	};
}
