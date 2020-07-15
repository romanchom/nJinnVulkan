#include <vector>
#include <map>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include <eigen3/Eigen/Dense>
#include <boost/iostreams/device/mapped_file.hpp>

#include "MeshLoader.hpp"
#include "MeshData.hpp"
#include "TypeConverter.hpp"
#include "VertexJudge.hpp"

namespace meshLoader {
	MeshLoader::MeshLoader() :
		_loaded(false),
		_shortIndexing(false)
	{}

	MeshLoader::~MeshLoader() {
		for (auto & attr : _attributes) {
			delete attr.typeConverter;
		}
	}

	void MeshLoader::setConverter(const int index, const TypeConverter * converter) {
		_attributes[index].typeConverter = converter;
	}

	template<typename T>
	void push_back(std::vector<T> & vector, T * data, size_t count) {
		T * end = data + count;
		do {
			vector.emplace_back(*data);
		} while (++data < end);
	}

	void MeshLoader::loadObj(const std::string & filename, bool verbose) {
		if (_loaded) throw std::runtime_error("Cannot load mesh into an already used object.");
		


		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> uvs;

		FILE * file = fopen(filename.c_str(), "r");

		if (file == 0) {
			throw std::runtime_error("Couldn't open file");
		}

		if (verbose) printf("Opened file %s.\n", filename.c_str());

		setvbuf(file, 0, _IOFBF, 1048576);

		char str[256];
		float tempFloat1, tempFloat2, tempFloat3;

		if (verbose) printf("Reading vertex positions.\n");

		fscanf(file, "%s", str);

		do {
			if (strcmp(str, "v") != 0) {
				if (strcmp(str, "vn") != 0 &&
					strcmp(str, "vt") != 0 &&
					strcmp(str, "f") != 0)
				{
					fgets(str, 256, file);
					continue;
				} else break;
			}

			fscanf(file, "%f %f %f", &tempFloat1, &tempFloat2, &tempFloat3);
			positions.push_back(tempFloat1);
			positions.push_back(tempFloat2);
			positions.push_back(tempFloat3);
			fgets(str, 256, file);

		} while (fscanf(file, "%s", str) > 0);

		if (verbose) {
			if (positions.size() != 0) printf("Read %llu vertex positions.\n", positions.size() / 3);
			else throw std::runtime_error("No vertex positions found. Aborting.");

			printf("Reading vertex normals.\n");
		}

		do {
			if (strcmp(str, "vn") != 0) {
				if (strcmp(str, "vt") != 0 &&
					strcmp(str, "f") != 0)
				{
					fgets(str, 256, file);
					continue;
				} else break;
			}

			fscanf(file, "%f %f %f", &tempFloat1, &tempFloat2, &tempFloat3);
			normals.push_back(tempFloat1);
			normals.push_back(tempFloat2);
			normals.push_back(tempFloat3);
			fgets(str, 256, file);

		} while (fscanf(file, "%s", str) > 0);

		if (verbose) {
			if (normals.size()) printf("Read %llu vertex normals.\n", normals.size() / 3);
			else throw std::runtime_error("No normals found. Aborting.");

			printf("Reading vertex texture coordinates.\n");
		}


		do {
			if (strcmp(str, "vt") != 0) {
				if (strcmp(str, "f") != 0)
				{
					fgets(str, 256, file);
					continue;
				} else break;
			}

			fscanf(file, "%f %f", &tempFloat1, &tempFloat2);
			uvs.push_back(tempFloat1);
			uvs.push_back(tempFloat2);
			fgets(str, 256, file);

		} while (fscanf(file, "%s", str) > 0);
		if (verbose) {
			if (uvs.size()) printf("Read %llu vertex texture coordinates.\n", uvs.size() / 2);
			else throw std::runtime_error("Found no texture coordinates. Aborting.");

			printf("Reading faces and optimizing.\n");
		}

		_attributes.emplace_back();
		_attributes.emplace_back();
		_attributes.emplace_back();

		std::vector<float> & finalPositions = _attributes[0].data;
		_attributes[0].componentCount = 3;
		std::vector<float> & finalUVs = _attributes[1].data;
		_attributes[1].componentCount = 2;
		std::vector<float> & finalNormals = _attributes[2].data;
		_attributes[2].componentCount = 3;

		{
			std::map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t> indexMap;
			std::pair<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t> tempIndicies;
			uint32_t & posIndex = std::get<0>(tempIndicies.first);
			uint32_t & uvIndex = std::get<1>(tempIndicies.first);
			uint32_t & normalIndex = std::get<2>(tempIndicies.first);
			uint32_t & currentIndex = tempIndicies.second;
			currentIndex = 0;
			do {
				if (strcmp(str, "f") != 0) {
					fgets(str, 256, file);
					continue;
				}
				for (uint32_t i = 0; i < 3; ++i) {
					fscanf(file, "%i/%i/%i", &posIndex, &uvIndex, &normalIndex);
					--posIndex;
					--uvIndex;
					--normalIndex;

					std::pair<std::map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t>::iterator, bool> result = indexMap.insert(tempIndicies);

					if (result.second) {
						_indicies.push_back(currentIndex++);

						push_back(finalPositions, &positions[posIndex * 3], 3);
						push_back(finalUVs, &uvs[uvIndex * 2], 2);
						push_back(finalNormals, &normals[normalIndex * 3], 3);
					} else _indicies.push_back(result.first->second);
				}
			} while (fscanf(file, "%s", str) > 0);
		}

		if (verbose) printf("Read %llu faces.\n", _indicies.size() / 3);
		if (verbose) printf("File read succesfully.\n");

		fclose(file);
		_loaded = true;

		_shortIndexing = (((finalPositions.size()) >> 16) == 0);
	}

	namespace bi = boost::iostreams;

	void meshLoader::MeshLoader::save(const std::string & fileName)
	{

		size_t indexTypeSize = _shortIndexing ? 2 : 4;
		size_t indexDataSize = _indicies.size() * indexTypeSize;
		/*size_t indexPadding = indexDataSize;
		indexDataSize += 15;
		indexDataSize &= ~15; // roundup to multiple of 16, a.k.a. allign to 4 floats
		indexPadding = indexDataSize - indexPadding;*/

		size_t vertexStride = 0;
		size_t vertexDataSize = 0;
		size_t vertexCount = _attributes[0].data.size() / _attributes[0].componentCount;
		for (auto & attr : _attributes) {
			vertexStride += attr.typeConverter->size() * attr.componentCount;
		}
		vertexDataSize = vertexStride * vertexCount;
		/*vertexDataSize += 15;
		vertexDataSize &= ~15; // roundup to multiple of 16, a.k.a. allign to 4 floats*/
		size_t totalSize = indexDataSize + vertexDataSize + 
			sizeof(vbm::Header) + sizeof(vbm::VertexStream) + sizeof(vbm::VertexAttribute) * _attributes.size();


		bi::mapped_file_params params;
		params.new_file_size = totalSize;
		params.length = totalSize;
		params.offset = 0;
		params.path = fileName;

		bi::mapped_file_sink file(params);
		char * p = file.data();

		vbm::Header & header = *reinterpret_cast<vbm::Header *>(p);
		p += sizeof(vbm::Header);
		header.magickNumber = vbm::Header::magickNumberValue;
		header.indexTypeSize = (uint32_t) indexTypeSize;
		header.indiciesCount = (uint32_t) _indicies.size();
		header.totalDataSize = (uint32_t) (indexDataSize + vertexDataSize);
		header.vertexCount = (uint32_t) _attributes[0].data.size();
		header.vetexStreamCount = 1;

		const size_t attrCount = _attributes.size();

		vbm::VertexStream & stream = *reinterpret_cast<vbm::VertexStream *>(p);
		p += sizeof(vbm::VertexStream);
		stream.offsetFromBufferBegin = (uint32_t) indexDataSize;
		stream.stride = (uint32_t) vertexStride;
		stream.vertexAttributeCount = (uint32_t) attrCount;

		uint32_t offset = 0;
		for (size_t i = 0; i < attrCount; ++i) {
			auto & src = _attributes[i];
			vbm::VertexAttribute & dst = *reinterpret_cast<vbm::VertexAttribute *>(p);
			p += sizeof(vbm::VertexAttribute);
			dst.componentCount = src.componentCount;
			dst.offset = offset;
			dst.type = static_cast<uint32_t>(src.typeConverter->type());
			offset += (uint32_t) (src.typeConverter->size() * src.componentCount);
		}

		if (_shortIndexing) {
			for (auto ind : _indicies) {
				*reinterpret_cast<uint16_t *>(p) = static_cast<uint16_t>(ind);
				p += 2;
			}
		} else {
			for (auto ind : _indicies) {
				*reinterpret_cast<uint32_t *>(p) = ind;
				p += 4;
			}
		}

		for (uint32_t i = 0; i < vertexCount; ++i) {
			for (auto & attr : _attributes) {
				p += attr.typeConverter->convert(p, &attr.data[i * attr.componentCount], attr.componentCount);
			}
		}
	}

	void MeshLoader::calculateTangent() {
		assert(_attributes.size() == 3);
		_attributes.emplace_back();

		VertexAttribute & tangentAttr = _attributes[3];
		tangentAttr.componentCount = 3;
		tangentAttr.data.resize(_attributes[2].data.size());

		Eigen::Vector3f * tangents = (Eigen::Vector3f *) tangentAttr.data.data();
		const Eigen::Vector3f * positions = (const Eigen::Vector3f *) _attributes[0].data.data();
		const Eigen::Vector2f * uvs = (const Eigen::Vector2f *) _attributes[1].data.data();

		const uint32_t indiciesCount = (uint32_t) _indicies.size();
		const uint32_t verticiesCount = (uint32_t) (tangentAttr.data.size() / 3);

		for (uint32_t i = 0; i < indiciesCount; i += 3) {
			const uint32_t ind[3] = { _indicies[i], _indicies[i + 1], _indicies[i + 2] };

			Eigen::Vector3f q[3];
			Eigen::Vector2f st[3];
			for (uint32_t j = 0; j < 3; ++j) {
				q[j] = positions[ind[(j + 1) % 3]] - positions[ind[j]];
				q[j].normalize();
				st[j] = uvs[ind[(j + 1) % 3]] - uvs[ind[j]];
			}

			Eigen::Matrix<float, 2, 3> tanBitan, qMat;
			Eigen::Matrix2f stMat;

			for (uint32_t j = 0; j < 3; ++j) {
				uint32_t jpo = (j + 1) % 3;
				qMat << q[j][0], -q[jpo][0], q[j][1], -q[jpo][1], q[j][2], -q[jpo][2];
				stMat << st[j][0], -st[jpo][0], st[j][1], -st[jpo][1];
				tanBitan = stMat.inverse() * qMat;
				Eigen::Vector3f tangent;
				for (uint32_t k = 0; k < 3; ++k) tangent[k] = tanBitan(0, k);
				tangent.normalize();
				tangent *= acos(q[j].dot(-q[jpo]));
				tangents[ind[j]] += tangent;
			}
		}

		Eigen::Vector3f * normals = (Eigen::Vector3f *) _attributes[2].data.data();

		for (uint32_t i = 0; i < verticiesCount; ++i) {
			Eigen::Vector3f biTan = normals[i].cross(tangents[i]);
			biTan.normalize();
			tangents[i] = biTan.cross(normals[i]).normalized();
		}
	}

	void MeshLoader::convertToDX()
	{
		// flip texture coordinate v
		{
			std::vector<float> & vec = _attributes[1].data;
			for (float * v = vec.data() + 1, *end = vec.data() + vec.size(); v < end; v += 2) {
				*v = 1.0f - *v;
			}
		}
	}

	void MeshLoader::swapYZ()
	{
		{
			std::vector<float> & vec = _attributes[0].data;
			for (float * v = vec.data() + 1, *end = vec.data() + vec.size(); v < end; v += 3) {
				std::swap(v[0], v[1]);
				v[0] *= -1;
			}
		}
		{
			std::vector<float> & vec = _attributes[2].data;
			for (float * v = vec.data() + 1, *end = vec.data() + vec.size(); v < end; v += 3) {
				std::swap(v[0], v[1]);
				v[0] *= -1;
			}
		}
	}

	typedef int scoreType;
	static const int scoreMultiplier = 1 << 24;

	struct VertexData
	{
		scoreType score;
		union {
			uint32_t * activeFaceListStart;
			uint32_t activeFaceListStartInt;
		};
		uint32_t activeFaceListSize;
		uint32_t cachePos;
		VertexData() : score(0), activeFaceListStartInt(0), activeFaceListSize(0), cachePos(INT_MAX) { }
	};

	void MeshLoader::optimizeForCache(uint32_t cacheSize, bool verbose) {
		if (verbose) printf("Optimizing mesh using cache size = %d.\n", cacheSize);
		uint32_t indiciesCount = (uint32_t) _indicies.size();
		uint32_t verticiesCount = (uint32_t) (_attributes[0].data.size() / _attributes[0].componentCount);
		uint32_t * const indicies = _indicies.data();


		VertexData * vertexDataArray = new VertexData[verticiesCount];

		for (uint32_t i(0); i < indiciesCount; ++i) {
			uint32_t index = indicies[i];
			++(vertexDataArray[index].activeFaceListSize);
		}

		uint32_t maxSharedVerticies(0);
		for (uint32_t i(0); i < verticiesCount; ++i) {
			maxSharedVerticies = std::max(vertexDataArray[i].activeFaceListSize, maxSharedVerticies);
		}

		VertexJudge<scoreType, scoreMultiplier> judge(cacheSize, maxSharedVerticies);

		uint32_t currentActiveFaceListPos(0);
		for (uint32_t i(0); i < verticiesCount; ++i) {
			VertexData& vertexData = vertexDataArray[i];
			vertexData.activeFaceListStartInt = currentActiveFaceListPos;
			currentActiveFaceListPos += vertexData.activeFaceListSize;
			vertexData.score = judge.getScore(INT_MAX, vertexData.activeFaceListSize);
			vertexData.activeFaceListSize = 0;
		}

		uint32_t * activeFaceList = new uint32_t[currentActiveFaceListPos];

		for (uint32_t i(0); i < verticiesCount; ++i) {
			VertexData& vertexData = vertexDataArray[i];
			vertexData.activeFaceListStart = activeFaceList + vertexData.activeFaceListStartInt;
		}

		for (uint32_t i(0); i < indiciesCount; i += 3) {
			for (uint32_t j(0); j < 3; ++j) {
				uint32_t index = indicies[i + j];
				VertexData& vertexData = vertexDataArray[index];
				vertexData.activeFaceListStart[vertexData.activeFaceListSize] = i;
				vertexData.activeFaceListSize++;
			}
		}


		bool * processedFaceList = new bool[indiciesCount];
		memset(processedFaceList, 0, indiciesCount * sizeof(bool));

		uint32_t * newIndicies = new uint32_t[indiciesCount];
		uint32_t * vertexCache = new uint32_t[(cacheSize + 3) * 2];

		uint32_t * cache0 = vertexCache;
		uint32_t * cache1 = vertexCache + (cacheSize + 3);

		uint32_t entriesInCache0(0);

		uint32_t bestFace(0);
		scoreType bestScore(-scoreMultiplier);

		const scoreType maxValenceScore = judge.getScore(INT_MAX, 1) * 3;

		if (verbose) printf("Triangle reordering:\n");

		for (uint32_t i(0); i < indiciesCount; i += 3) {
			if (verbose && i % 999 == 0) printf("%f%%.\r", i * 100.0 / indiciesCount);
			if (bestScore < 0) {
				for (uint32_t j(0); j < indiciesCount; j += 3) {
					if (!processedFaceList[j]) {
						scoreType faceScore(0);
						for (uint32_t k(0); k < 3; ++k) {
							uint32_t index = indicies[j + k];
							VertexData & vertexData = vertexDataArray[index];
							faceScore += vertexData.score;
						}

						if (faceScore > bestScore) {
							bestScore = faceScore;
							bestFace = j;
							if (bestScore >= maxValenceScore) break;
						}
					}
				}
			}

			processedFaceList[bestFace] = true;
			uint32_t entriesInCache1(0);

			for (uint32_t v(0); v < 3; ++v) {
				uint32_t index = indicies[bestFace + v];
				newIndicies[i + v] = index;

				VertexData & vertexData = vertexDataArray[index];
				if (vertexData.cachePos >= entriesInCache1) {
					vertexData.cachePos = entriesInCache1;
					cache1[entriesInCache1++] = index;

					if (vertexData.activeFaceListSize == 1) {
						--vertexData.activeFaceListSize;
						continue;
					}
				}

				uint32_t* begin = vertexData.activeFaceListStart;
				uint32_t* end = vertexData.activeFaceListStart + vertexData.activeFaceListSize;
				uint32_t* it = std::find(begin, end, bestFace);
				std::swap(*it, *(end - 1));
				--vertexData.activeFaceListSize;
				vertexData.score = judge.getScore(vertexData.cachePos, vertexData.activeFaceListSize);
			}

			// move the rest of the old verts in the cache down and compute their new scores
			for (uint32_t c0(0); c0 < entriesInCache0; ++c0)
			{
				uint32_t index = cache0[c0];
				VertexData& vertexData = vertexDataArray[index];

				if (vertexData.cachePos > entriesInCache1)
				{
					vertexData.cachePos = entriesInCache1;
					cache1[entriesInCache1++] = index;
					vertexData.score = judge.getScore(vertexData.cachePos, vertexData.activeFaceListSize);
				}
			}

			// find the best scoring triangle in the current cache (including up to 3 that were just evicted)
			bestScore = -scoreMultiplier;
			for (uint32_t c1(0); c1 < entriesInCache1; ++c1)
			{
				uint32_t index = cache1[c1];
				VertexData& vertexData = vertexDataArray[index];
				vertexData.cachePos = INT_MAX;
				for (uint32_t j(0); j < vertexData.activeFaceListSize; ++j)
				{
					uint32_t face = vertexData.activeFaceListStart[j];
					scoreType faceScore = 0;
					for (uint32_t v(0); v < 3; v++)
					{
						faceScore += vertexDataArray[indicies[face + v]].score;
					}

					if (faceScore > bestScore)
					{
						bestScore = faceScore;
						bestFace = face;
					}
				}
			}

			std::swap(cache0, cache1);
			entriesInCache0 = std::min(entriesInCache1, cacheSize);
		}

		delete[] vertexDataArray;
		delete[] processedFaceList;
		delete[] activeFaceList;
		delete[] vertexCache;


		if (verbose) printf("Triangle reordering done.\n");
		if (verbose) printf("Reordering verticies.\n");


		uint32_t attrCount = (uint32_t) _attributes.size();
		uint32_t * mapping = new uint32_t[verticiesCount * 2];
		uint32_t * revMapping = mapping + verticiesCount;

		for (uint32_t i = 0; i < verticiesCount; ++i) {
			mapping[i] = i;
			revMapping[i] = i;
		}

		uint32_t currentIndex = 0;
		for (uint32_t i = 0; i < indiciesCount; ++i) {
			uint32_t index = newIndicies[i];
			if (revMapping[index] >= currentIndex) {
				uint32_t location = revMapping[index];
				for (uint32_t j = 0; j < attrCount; ++j) {
					VertexAttribute & attr = _attributes[j];
					for (uint32_t k = 0; k < attr.componentCount; ++k) {
						std::swap(attr.data[location * attr.componentCount + k], attr.data[currentIndex * attr.componentCount + k]);
					}
				}
				revMapping[mapping[currentIndex]] = location;
				revMapping[index] = currentIndex;
				std::swap(mapping[location], mapping[currentIndex]);
				++currentIndex;
			}
			indicies[i] = revMapping[index];
		}

		delete[] newIndicies;
		delete[] mapping;

		if (verbose) printf("Reordering verticies done.\n");
	}
}