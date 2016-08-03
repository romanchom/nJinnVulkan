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
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(false);

		blendState
			.setAttachmentCount(1)
			.setPAttachments(&state);

		vk::DescriptorSetLayoutBinding bindings[2];
		vk::DescriptorSetLayoutCreateInfo descInfo;
		// TODO include global per frame uniforms
		bindings[0]
			.setBinding(0)
			.setDescriptorCount(3)
			.setDescriptorType(vk::DescriptorType::eSampledImage)
			.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		bindings[1]
			.setBinding(1)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		descInfo
			.setBindingCount(2)
			.setPBindings(bindings);

		mMaterialAllocator.mLayout = context->dev().createDescriptorSetLayout(descInfo);

		bindings[0]
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		descInfo
			.setBindingCount(1)
			.setPBindings(bindings);

		mObjectAllocator.mLayout = context->dev().createDescriptorSetLayout(descInfo);


		vk::DescriptorSetLayout layouts[] = {
			mMaterialAllocator.mLayout,
			mObjectAllocator.mLayout
		};

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.setPSetLayouts(layouts)
			.setSetLayoutCount(2);

		mLayout = context->dev().createPipelineLayout(layoutInfo);

		mPoolSizes[0]
			.setType(vk::DescriptorType::eSampledImage)
			.setDescriptorCount(30);
		mPoolSizes[1]
			.setType(vk::DescriptorType::eSampler)
			.setDescriptorCount(10);
		mPoolSizes[2]
			.setType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(10);

		mMaterialAllocator.mPoolCreateInfo
			.setMaxSets(10)
			.setPoolSizeCount(2)
			.setPPoolSizes(mPoolSizes);

		mObjectAllocator.mPoolCreateInfo
			.setMaxSets(10)
			.setPoolSizeCount(1)
			.setPPoolSizes(mPoolSizes + 2);

		if (vertexShader) stages[mStageCount++] = *vertexShader;
		if (fragmentShader) stages[mStageCount++] = *fragmentShader;
	}

	MaterialFamily::~MaterialFamily()
	{
		context->dev().destroyPipelineLayout(mLayout);
	}

	Material * MaterialFamily::instantiate()
	{
		Material * ret = new Material();
		ret->mMaterialFamily = this;
		ret->mDescriptorSet = mMaterialAllocator.allocateDescriptorSet();
		return ret;
	}

	void MaterialFamily::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		info
			.setLayout(mLayout)
			.setStageCount(mStageCount)
			.setPStages(stages)
			.setPColorBlendState(&blendState);
	}

	MaterialFamily::DescriptorAllocator::~DescriptorAllocator() {
		context->dev().destroyDescriptorSetLayout(mLayout);
		for (auto it : mPools) {
			context->dev().destroyDescriptorPool(it);
		}
	}

	vk::DescriptorSet MaterialFamily::DescriptorAllocator::allocateDescriptorSet()
	{
		vk::DescriptorSet ret = nullptr;
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo
			.setDescriptorSetCount(1)
			.setPSetLayouts(&mLayout);

		uint32_t  tryCount = (uint32_t) mPools.size();
		while (tryCount > 0) {
			auto it = mPools.begin();
			allocInfo.setDescriptorPool(*it);
			if (vk::Result::eSuccess == context->dev().allocateDescriptorSets(&allocInfo, &ret)) {
				return ret;
			} else {
				// move full pool to the back
				mPools.splice(mPools.end(), mPools, it);
				--tryCount;
			}
		}
		vk::DescriptorPool pool = context->dev().createDescriptorPool(mPoolCreateInfo);
		mPools.push_back(pool);
		allocInfo.setDescriptorPool(pool);
		context->dev().allocateDescriptorSets(&allocInfo, &ret);
		return ret;
	}

	
}
