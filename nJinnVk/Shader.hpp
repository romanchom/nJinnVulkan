#pragma once

#include <vulkan.hpp>
#include "TrackedResource.hpp"


namespace nJinn {
	class Shader : public TrackedResource<Shader, std::pair<std::string, vk::ShaderStageFlagBits>>{
	public:
		Shader(const std::pair<std::string, vk::ShaderStageFlagBits> & name);
		operator const vk::PipelineShaderStageCreateInfo &() const { return shaderInfo; }
	private:
		vk::PipelineShaderStageCreateInfo shaderInfo;
		vk::ShaderModule shaderModule;
	};
}