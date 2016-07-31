#include "stdafx.hpp"
#include "Shader.hpp"

#include <boost/iostreams/device/mapped_file.hpp>
#include "Context.hpp"

namespace nJinn {
	using namespace boost::iostreams;

	Shader::Shader(const std::pair<std::string, vk::ShaderStageFlagBits>& name)
	{
		mapped_file_source file(name.first);
		if (!file.is_open()) throw std::runtime_error("Couldn't open shader file " + name.first);
		file.begin();

		vk::ShaderModuleCreateInfo moduleInfo;
		moduleInfo
			.setCodeSize(file.size())
			.setPCode(reinterpret_cast<const uint32_t *>(file.data()));

		shaderModule = Context::dev().createShaderModule(moduleInfo);

		shaderInfo
			.setStage(name.second)
			.setModule(shaderModule)
			.setPName("main");
	}

	Shader::~Shader()
	{
		Context::dev().destroyShaderModule(shaderModule);
	}
}