#pragma once

#include <vulkan.hpp>

#include "TrackedResource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class MaterialFamily : public TrackedResource<MaterialFamily> {
		Shader::p vertexShader;
		Shader::p fragmentShader;

		vk::DescriptorSetLayout materialDescriptorSetLayout;
		vk::DescriptorSetLayout objectDescriptorSetLayout;
		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo blendState;
		vk::PipelineColorBlendAttachmentState state;
		size_t mStageCount;
		vk::PipelineShaderStageCreateInfo stages[2];
	public:
		MaterialFamily(const std::string & name);
		class Material * instantiate();
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		vk::PipelineLayout layout() { return mLayout; }
	};
}