#include "stdafx.hpp"
#include "Shader.hpp"

#include <boost/iostreams/device/mapped_file.hpp>
#include <yaml-cpp/yaml.h>
#include "YamlUtility.hpp"
#include "Context.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	using namespace boost::iostreams;
	using namespace YAML;

	Shader::Shader()
	{}

	Shader::~Shader()
	{
		context->dev().destroyShaderModule(mShaderModule);
	}

	void Shader::load(const std::string & fileName)
	{
		Node root = LoadFile(fileName);
		std::string shaderSourceFileName = root["source"].as<std::string>();
		vk::ShaderStageFlagBits shaderStage = root["type"].as<vk::ShaderStageFlagBits>();
		mEntryPoint = root["entryPoint"].as<std::string>();

		mapped_file_source file(shaderSourceFileName);

		if (!file.is_open()) throw std::runtime_error("Couldn't open shader file " + fileName);
		file.begin();

		vk::ShaderModuleCreateInfo moduleInfo;
		moduleInfo
			.setCodeSize(file.size())
			.setPCode(reinterpret_cast<const uint32_t *>(file.data()));

		mShaderModule = context->dev().createShaderModule(moduleInfo);

		mShaderInfo
			.setStage(shaderStage)
			.setModule(mShaderModule)
			.setPName(mEntryPoint.c_str());

		finishedLoading();
	}
}