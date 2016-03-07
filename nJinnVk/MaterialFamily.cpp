#include "stdafx.hpp"
#include "MaterialFamily.hpp"

#include "Context.hpp"
#include "Material.hpp"
#include "PipelineFactory.hpp"

namespace nJinn {
	MaterialFamily::MaterialFamily(const std::string & name) : 
		mStageCount(0)
	{
		// TODO read shader names and descriptor sets from a file
		vertexShader = Shader::load({ "shaders/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex });
		fragmentShader = Shader::load({ "shaders/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment });
		
		state
			.colorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.blendEnable(false);

		blendState
			.attachmentCount(1)
			.pAttachments(&state);

		vk::DescriptorSetLayoutBinding bindings[2];
		vk::DescriptorSetLayoutCreateInfo descInfo;

		bindings[0]
			.binding(0)
			.descriptorCount(3)
			.descriptorType(vk::DescriptorType::eSampledImage)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		bindings[1]
			.binding(1)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eSampler)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		descInfo
			.bindingCount(2)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(Context::dev(), &descInfo, nullptr, &materialDescriptorSetLayout));

		bindings[0]
			.binding(0)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBuffer)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		descInfo
			.bindingCount(1)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(Context::dev(), &descInfo, nullptr, &objectDescriptorSetLayout));

		vk::DescriptorSetLayout layouts[] = {
			materialDescriptorSetLayout,
			objectDescriptorSetLayout
		};

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.pSetLayouts(layouts)
			.setLayoutCount(2);

		dc(vk::createPipelineLayout(Context::dev(), &layoutInfo, nullptr, &mLayout));

		if (vertexShader) stages[mStageCount++] = *vertexShader;
		if (fragmentShader) stages[mStageCount++] = *fragmentShader;

	}

	Material * MaterialFamily::instantiate()
	{
		Material * ret = new Material();
		ret->mMaterialFamily = this;
		return ret;
	}



	void MaterialFamily::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		info
			.layout(mLayout)
			.stageCount(mStageCount)
			.pStages(stages)
			.pColorBlendState(&blendState);
	}
}
