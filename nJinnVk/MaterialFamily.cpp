#include "stdafx.hpp"
#include "MaterialFamily.hpp"

#include <yaml-cpp/yaml.h>

#include "Context.hpp"
#include "Material.hpp"
#include "PipelineFactory.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	using namespace YAML;

	void MaterialFamily::load(const std::string & name)
	{
		Node root = LoadFile(name);
		{
			Node shaders = root["shaders"];
			assert(shaders.IsSequence());
			int i = 0;
			for (auto & node : shaders) {
				mShaders[i] = resourceManager->get<Shader>(shaders[i].as<std::string>(), true);
				++i;
			}
		}
		
		// TODO descriptor sets from a file
		mBlendAttachmentState
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(false);

		mBlendState
			.setAttachmentCount(1)
			.setPAttachments(&mBlendAttachmentState);

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

		for (int i = 0; i < shaderCount; ++i) {
			if (mShaders[i]) {
				mShaderStages[mStageCount++] = mShaders[i]->shaderInfo();
			}
		}
	}

	MaterialFamily::MaterialFamily() :
		mStageCount(0)
	{}

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
			.setPStages(mShaderStages)
			.setPColorBlendState(&mBlendState);
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
