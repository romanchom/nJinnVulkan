#pragma once

#include <vulkan.hpp>

#include "TrackedResource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class Material /*: public TrackedResource<Material> */{
		Shader::p vertexShader;
		Shader::p fragmentShader;

		// blend state
		vk::PipelineColorBlendStateCreateInfo blendState;
		vk::PipelineColorBlendAttachmentState state;
		vk::PipelineShaderStageCreateInfo stages[2];
	public:
		Material(const std::string & vert, const std::string & frag, bool transparent);
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
	};
}