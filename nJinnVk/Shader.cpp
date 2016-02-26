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
			.codeSize(file.size())
			.pCode(reinterpret_cast<const uint32_t *>(file.data()));

		dc(vk::createShaderModule(Context::dev(), &moduleInfo, nullptr, &shaderModule));

		shaderInfo
			.stage(name.second)
			.module(shaderModule)
			.pName("main");
	}

	Shader::~Shader()
	{
		vk::destroyShaderModule(Context::dev(), shaderModule, nullptr);
	}
}