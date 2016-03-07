#pragma once

#include <vulkan.hpp>

#include "TrackedResource.hpp"
#include "MemoryAllocation.hpp"

namespace nJinn {
	class Mesh : public TrackedResource<Mesh> {
	public:
		Mesh(const std::string & name);
		~Mesh();
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		void bind(vk::CommandBuffer cmdbuf);
		void draw(vk::CommandBuffer cmdbuf);
	private:
		vk::Buffer buffer;
		MemoryAllocation bufferMemory;
		vk::PipelineVertexInputStateCreateInfo vertexDataLayout;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		vk::PipelineTessellationStateCreateInfo tessInfo;
		vk::VertexInputBindingDescription veretxBindingDescription[2];
		uint64_t vertexBufferOffsets[2];
		vk::VertexInputAttributeDescription vertexAttributeDescriptions[10];
		uint32_t bindingCount;
		uint32_t indexCount;
		vk::IndexType indexType;
	};
}