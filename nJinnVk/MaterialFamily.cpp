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
		// TODO include global per frame uniforms
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

		dc(vk::createDescriptorSetLayout(Context::dev(), &descInfo, nullptr, &mMaterialAllocator.mLayout));

		bindings[0]
			.binding(0)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		descInfo
			.bindingCount(1)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(Context::dev(), &descInfo, nullptr, &mObjectAllocator.mLayout));


		vk::DescriptorSetLayout layouts[] = {
			mMaterialAllocator.mLayout,
			mObjectAllocator.mLayout
		};

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.pSetLayouts(layouts)
			.setLayoutCount(2);

		dc(vk::createPipelineLayout(Context::dev(), &layoutInfo, nullptr, &mLayout));

		mPoolSizes[0]
			.type(vk::DescriptorType::eSampledImage)
			.descriptorCount(30);
		mPoolSizes[1]
			.type(vk::DescriptorType::eSampler)
			.descriptorCount(10);
		mPoolSizes[2]
			.type(vk::DescriptorType::eUniformBufferDynamic)
			.descriptorCount(10);

		mMaterialAllocator.mPoolCreateInfo
			.maxSets(10)
			.poolSizeCount(2)
			.pPoolSizes(mPoolSizes);

		mObjectAllocator.mPoolCreateInfo
			.maxSets(10)
			.poolSizeCount(1)
			.pPoolSizes(mPoolSizes + 2);

		if (vertexShader) stages[mStageCount++] = *vertexShader;
		if (fragmentShader) stages[mStageCount++] = *fragmentShader;
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
			.layout(mLayout)
			.stageCount(mStageCount)
			.pStages(stages)
			.pColorBlendState(&blendState);
	}

	MaterialFamily::DescriptorAllocator::~DescriptorAllocator() {
		vk::destroyDescriptorSetLayout(Context::dev(), mLayout, nullptr);
		for (auto it : mPools) {
			vk::destroyDescriptorPool(Context::dev(), it, nullptr);
		}
	}

	vk::DescriptorSet MaterialFamily::DescriptorAllocator::allocateDescriptorSet()
	{
		vk::DescriptorSet ret = nullptr;
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo
			.descriptorSetCount(1)
			.pSetLayouts(&mLayout);

		int tryCount = mPools.size();
		while (tryCount > 0) {
			auto it = mPools.begin();
			allocInfo.descriptorPool(*it);
			if (vk::allocateDescriptorSets(Context::dev(), &allocInfo, &ret) == vk::Result::eVkSuccess) {
				return ret;
			} else {
				// move full pool to the back
				mPools.splice(mPools.end(), mPools, it);
				--tryCount;
			}
		}
		vk::DescriptorPool pool;
		dc(vk::createDescriptorPool(Context::dev(), &mPoolCreateInfo, nullptr, &pool));
		mPools.push_back(pool);
		allocInfo.descriptorPool(pool);
		dc(vk::allocateDescriptorSets(Context::dev(), &allocInfo, &ret));
		return ret;
	}

	
}
