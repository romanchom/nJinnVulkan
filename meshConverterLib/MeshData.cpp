#include "MeshData.hpp"

#include <fstream>

namespace meshLoader {

	MeshData::MeshData(const std::string & fileName) :
		file(fileName),
		attributeCount_(0)
	{
		const char * p = file.data();
		header = reinterpret_cast<const vbm::Header *>(p);
		p += sizeof(vbm::Header);

		if (header->magickNumber != vbm::Header::magickNumberValue)
			throw std::runtime_error("File format does not match.");

		vertexStreams = new const vbm::VertexStream *[vertexStreamCount()];
		vertexAttributes = new const vbm::VertexAttribute *[vertexStreamCount()];

		for (size_t i = 0; i < vertexStreamCount(); ++i) {
			vertexStreams[i] = reinterpret_cast<const vbm::VertexStream *>(p);
			p += sizeof(vbm::VertexStream);
			attributeCount_ += vertexStreams[i]->vertexAttributeCount;
			vertexAttributes[i] = reinterpret_cast<const vbm::VertexAttribute *>(p);
			p += sizeof(vbm::VertexAttribute) * vertexStreams[i]->vertexAttributeCount;
		}
		dataStart = p;
	}


	MeshData::~MeshData()
	{
		delete[] vertexAttributes;
		delete[] vertexStreams;
	}
}
