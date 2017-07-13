#include "stdafx.hpp"
#include "MaterialFamily.hpp"

#include "YamlUtility.hpp"

#include "Context.hpp"
#include "Material.hpp"
#include "PipelineFactory.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	using namespace YAML;

	void MaterialFamily::load(const std::string & name)
	{
		Node root = LoadFile(name);
		if(Node shaders = root["shaders"]){
			assert(shaders.IsSequence());
			int i = 0;
			for (auto & node : shaders) {
				mShaders[i] = resourceManager->get<Shader>(shaders[i].as<std::string>(), ResourceLoadPolicy::Immediate);
				++i;
			}
		}
		else {
			throw std::runtime_error("Missing required parameter: shaders");
		}
		
		uint32_t attachmentCount = root["outputCount"].as<uint32_t>();

		for (int i = 0; i < attachmentCount; ++i) {
			mBlendAttachmentState[i]
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
				.setBlendEnable(false);
		}

		mBlendState
			.setAttachmentCount(attachmentCount)
			.setPAttachments(mBlendAttachmentState);

		mMaterialAllocator.parseYAML(root["materialParams"]);
		mObjectAllocator.parseYAML(root["objectParams"]);

		if (Node depthNode = root["depth"]) {
			if (Node writeEnable = depthNode["write"]) {
				mDepthStencilInfo
					.setDepthTestEnable(1)
					.setDepthWriteEnable(1)
					.setDepthCompareOp(vk::CompareOp::eAlways);
			}
			if (Node compare = depthNode["compare"]) {
				mDepthStencilInfo
					.setDepthCompareOp(compare.as<vk::CompareOp>())
					.setDepthTestEnable(1);
			}
			if (Node bounds = depthNode["bounds"]) {
				mDepthStencilInfo.setDepthBoundsTestEnable(1);
				mDepthStencilInfo.setMinDepthBounds(bounds[0].as<float>());
				mDepthStencilInfo.setMaxDepthBounds(bounds[1].as<float>());
			}
		}

		vk::DescriptorSetLayout layouts[] = {
			mMaterialAllocator.layout(),
			mObjectAllocator.layout()
		};

		// TODO include global layouts
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.setPSetLayouts(layouts)
			.setSetLayoutCount(2);

		mLayout = context->dev().createPipelineLayout(layoutInfo);

		for (int i = 0; i < shaderCount; ++i) {
			if (mShaders[i]) {
				mShaderStages[mStageCount++] = mShaders[i]->shaderInfo();
			}
		}

		finishedLoading();
	}

	MaterialFamily::MaterialFamily() :
		mStageCount(0)
	{
		mRasterisationInfo.setLineWidth(1.0f);
	}

	MaterialFamily::~MaterialFamily()
	{
		context->dev().destroyPipelineLayout(mLayout);
	}

	void MaterialFamily::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		info
			.setLayout(mLayout)
			.setStageCount(mStageCount)
			.setPStages(mShaderStages)
			.setPColorBlendState(&mBlendState)
			.setPRasterizationState(&mRasterisationInfo)
			.setPDepthStencilState(&mDepthStencilInfo);
	}

	
}
