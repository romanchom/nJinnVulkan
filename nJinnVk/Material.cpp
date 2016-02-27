#include "stdafx.hpp"
#include "Material.hpp"

#include <regex>

namespace nJinn {
	Material::Material(const std::string & vert, const std::string & frag, bool transparent)
	{
		vertexShader = Shader::load({ vert, vk::ShaderStageFlagBits::eVertex });
		fragmentShader = Shader::load({ frag, vk::ShaderStageFlagBits::eFragment });
		
		state
			.colorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.blendEnable(false);

		blendState
			.attachmentCount(1)
			.pAttachments(&state);
	}

	void Material::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		size_t stageCount = 0;

		if (vertexShader) stages[stageCount++] = *vertexShader;
		if (fragmentShader) stages[stageCount++] = *fragmentShader;

		info
			.stageCount(stageCount)
			.pStages(stages)
			.pColorBlendState(&blendState);
	}
}
