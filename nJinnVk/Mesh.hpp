#pragma once

#include <vulkan.hpp>
#include "TrackedResource.hpp"

namespace nJinn {
	class Mesh : public TrackedResource<Mesh> {
	public:
		Mesh(const std::string & name);
		~Mesh();
	private:
		vk::Buffer buffer;
		vk::DeviceMemory deviceMemory;
		vk::PipelineVertexInputStateCreateInfo vertexDataLayout;
		vk::VertexInputBindingDescription veretxBindingDescription[2];
		uint32_t vertexBufferOffsets[2];
		vk::VertexInputAttributeDescription vertexAttributeDescriptions[10];
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	};
}