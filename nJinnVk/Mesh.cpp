#include "stdafx.hpp"
#include "Mesh.hpp"

#include "Context.hpp"
#include <MeshData.hpp>

#include "ResourceUploader.hpp"

namespace nJinn {
	typedef vk::Format f;

	const f formatTable[19][4] = {
		{f::eR8Unorm,		f::eR8G8Unorm,		f::eR8G8B8Unorm,		f::eR8G8B8A8Unorm		},	// 8 UN
		{f::eR8Snorm,		f::eR8G8Snorm,		f::eR8G8B8Snorm,		f::eR8G8B8A8Snorm		},	// 8 SN
		{f::eR8Uscaled,		f::eR8G8Uscaled,	f::eR8G8B8Uscaled,		f::eR8G8B8A8Uscaled		},	// 8 US
		{f::eR8Sscaled,		f::eR8G8Sscaled,	f::eR8G8B8Sscaled,		f::eR8G8B8A8Sscaled		},	// 8 SS
		{f::eR8Uint,		f::eR8G8Uint,		f::eR8G8B8Uint,			f::eR8G8B8A8Uint		},	// 8 UINT
		{f::eR8Sint,		f::eR8G8Sint,		f::eR8G8B8Sint,			f::eR8G8B8A8Sint		},	// 8 SINT
		{f::eR16Unorm,		f::eR16G16Unorm,	f::eR16G16B16Unorm,		f::eR16G16B16A16Unorm	},	// 16 UN
		{f::eR16Snorm,		f::eR16G16Snorm,	f::eR16G16B16Snorm,		f::eR16G16B16A16Snorm	},	// 16 SN
		{f::eR16Uscaled,	f::eR16G16Uscaled,	f::eR16G16B16Uscaled,	f::eR16G16B16A16Uscaled	},	// 16 US
		{f::eR16Sscaled,	f::eR16G16Sscaled,	f::eR16G16B16Sscaled,	f::eR16G16B16A16Sscaled	},	// 16 SS
		{f::eR16Uint,		f::eR16G16Uint,		f::eR16G16B16Uint,		f::eR16G16B16A16Uint	},	// 16 INT
		{f::eR16Sint,		f::eR16G16Sint,		f::eR16G16B16Sint,		f::eR16G16B16A16Sint	},	// 16 UINT
		{f::eR16Sfloat,		f::eR16G16Sfloat,	f::eR16G16B16Sfloat,	f::eR16G16B16A16Sfloat	},	// 16 FLOAT
		{f::eR32Uint,		f::eR32G32Uint,		f::eR32G32B32Uint,		f::eR32G32B32A32Uint	},	// 32 INT
		{f::eR32Sint,		f::eR32G32Sint,		f::eR32G32B32Sint,		f::eR32G32B32A32Sint	},	// 32 UINT
		{f::eR32Sfloat,		f::eR32G32Sfloat,	f::eR32G32B32Sfloat,	f::eR32G32B32A32Sfloat	},	// 32 FLOAT
		{f::eR64Uint,		f::eR64G64Uint,		f::eR64G64B64Uint,		f::eR64G64B64A64Uint	},	// 64 INT
		{f::eR64Sint,		f::eR64G64Sint,		f::eR64G64B64Sint,		f::eR64G64B64A64Sint	},	// 64 UINT
		{f::eR64Sfloat,		f::eR64G64Sfloat,	f::eR64G64B64Sfloat,	f::eR64G64B64A64Sfloat	},	// 64 FLOAT
	};

	Mesh::Mesh(const std::string & name)
	{
		meshLoader::MeshData meshData(name);

		size_t totalSize = meshData.totalDataSize();

		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.size(totalSize)
			.usage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

		vk::createBuffer(Context::dev(), &bufferInfo, nullptr, &buffer);

		vk::MemoryRequirements memReq;
		vk::getBufferMemoryRequirements(Context::dev(), buffer, memReq);

		bufferMemory.allocate(memReq.size());

		dc(vk::bindBufferMemory(Context::dev(), buffer, bufferMemory, 0));
		size_t totalVertexAttributes = 0;
		
		for (size_t s = 0; s < meshData.vertexStreamCount(); ++s) {
			const vbm::VertexStream & stream = meshData.vertexStream(s);
			veretxBindingDescription[s]
				.binding(s)
				.inputRate(vk::VertexInputRate::eVertex)
				.stride(stream.stride);
			vertexBufferOffsets[0] = stream.offsetFromBufferBegin;
			uint32_t attributeCount = stream.vertexAttributeCount;
			for (int a = 0; a < attributeCount; ++a) {
				vk::VertexInputAttributeDescription & dst = vertexAttributeDescriptions[totalVertexAttributes++];
				const vbm::VertexAttribute & src = meshData.vertexAttribute(s, a);
				dst
					.binding(s)
					.location(a)
					.offset(src.offset)
					.format(formatTable[src.type][src.componentCount]);
			}
		}

		vertexDataLayout
			.vertexBindingDescriptionCount(meshData.vertexStreamCount())
			.pVertexBindingDescriptions(veretxBindingDescription)
			.vertexAttributeDescriptionCount(totalVertexAttributes)
			.pVertexAttributeDescriptions(vertexAttributeDescriptions);

		ResourceUploader::upload(meshData.data(), meshData.totalDataSize(), buffer);
	}

	Mesh::~Mesh()
	{
	}
}
