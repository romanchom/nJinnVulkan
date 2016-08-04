#pragma once

#include <vulkan.hpp>
#include <memory>
#include "Resource.hpp"


namespace nJinn {
	class Shader : public Resource{
	public:
		typedef std::shared_ptr<Shader> handle;
		Shader();
		~Shader();
		virtual void load(const std::string & fileName) override;
		const vk::PipelineShaderStageCreateInfo & shaderInfo() const { return mShaderInfo; }
	private:
		std::string mEntryPoint;
		vk::PipelineShaderStageCreateInfo mShaderInfo;
		vk::ShaderModule mShaderModule;
	};
}