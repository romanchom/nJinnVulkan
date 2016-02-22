#pragma once

#include <string>
#include "TypeConverter.hpp"
#include "MeshData.hpp"

namespace meshLoader {
	class MeshLoader {
	public:
		MeshLoader();
		~MeshLoader();
		void loadObj(const std::string & filename, bool verbose = false);
		//void toMeshData(MeshData & mesh);
		void save(const std::string & fileName);
		void setConverter(const int index, const TypeConverter * converter);
		void calculateTangent();
		void convertToDX();
		void swapYZ();
		void optimizeForCache(uint32_t cacheSize, bool verbose);
	private:
		bool _loaded;
		struct VertexAttribute {
			uint32_t componentCount;
			std::vector<float> data;
			const TypeConverter * typeConverter;
			VertexAttribute() : componentCount(0), typeConverter(nullptr) {}
		};
		std::vector<VertexAttribute> _attributes;
		std::vector<uint32_t> _indicies;
		bool _shortIndexing;
	};
}