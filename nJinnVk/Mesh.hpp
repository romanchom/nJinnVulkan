#pragma once

#include <vulkan.hpp>

#include "Resource.hpp"

#include "Memory.hpp"

namespace nJinn {
	class Mesh : public Resource {
	public:
		typedef std::shared_ptr<Mesh> handle;
		Mesh();
		~Mesh();
		virtual void load(const std::string & name) override;
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		void bind(vk::CommandBuffer cmdbuf);
		void draw(vk::CommandBuffer cmdbuf);
	private:
		vk::Buffer buffer;
		MemoryAllocation mMemory;
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