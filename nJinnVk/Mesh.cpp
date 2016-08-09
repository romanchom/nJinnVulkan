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


	Mesh::Mesh() :
		bindingCount(0),
		indexCount(0)
	{
	}

	Mesh::~Mesh() {
		context->dev().destroyBuffer(buffer);
	}

	void Mesh::load(const std::string & name)
	{
		meshLoader::MeshData meshData(name);

		size_t totalSize = meshData.totalDataSize();
		bindingCount = (uint32_t) meshData.vertexStreamCount();
		indexCount = meshData.indexCount();
		indexType = (meshData.indexSize() == 2) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;


		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.setSize(totalSize)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

		buffer = context->dev().createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = context->dev().getBufferMemoryRequirements(buffer);

		bufferMemory.allocate(memReq.size);

		context->dev().bindBufferMemory(buffer, bufferMemory, 0);
		uint32_t totalVertexAttributes = 0;
		
		for (uint32_t s = 0; s < meshData.vertexStreamCount(); ++s) {
			const vbm::VertexStream & stream = meshData.vertexStream(s);
			veretxBindingDescription[s]
				.setBinding(s)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(stream.stride);
			vertexBufferOffsets[0] = stream.offsetFromBufferBegin;
			uint32_t attributeCount = stream.vertexAttributeCount;
			for (uint32_t a = 0; a < attributeCount; ++a) {
				vk::VertexInputAttributeDescription & dst = vertexAttributeDescriptions[totalVertexAttributes++];
				const vbm::VertexAttribute & src = meshData.vertexAttribute(s, a);
				dst
					.setBinding(s)
					.setLocation(a)
					.setOffset(src.offset)
					.setFormat(formatTable[src.type][src.componentCount - 1]);
				int as = 5;
			}
		}

		vertexDataLayout
			.setVertexBindingDescriptionCount(meshData.vertexStreamCount())
			.setPVertexBindingDescriptions(veretxBindingDescription)
			.setVertexAttributeDescriptionCount(totalVertexAttributes)
			.setPVertexAttributeDescriptions(vertexAttributeDescriptions);

		inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

		tessInfo.setPatchControlPoints(3);

		resourceUploader->upload(meshData.data(), meshData.totalDataSize(), buffer);

		finishedLoading();
	}

	void Mesh::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		info
			.setPVertexInputState(&vertexDataLayout)
			.setPInputAssemblyState(&inputAssemblyInfo)
			.setPTessellationState(&tessInfo);
	}

	void Mesh::bind(vk::CommandBuffer cmdbuf)
	{
		vk::Buffer b[] = {
			buffer, buffer
		};
		cmdbuf.bindIndexBuffer(buffer, 0, indexType);
		cmdbuf.bindVertexBuffers(0, bindingCount, b, vertexBufferOffsets);
	}
	void Mesh::draw(vk::CommandBuffer cmdbuf)
	{
		cmdbuf.drawIndexed(indexCount, 1, 0, 0, 1);
	}
}
