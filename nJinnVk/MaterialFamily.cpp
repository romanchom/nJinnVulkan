#include "stdafx.hpp"
#include "MaterialFamily.hpp"


#include "Context.hpp"
#include "Material.hpp"
#include "PipelineFactory.hpp"
#include "ResourceManager.hpp"
#include "Hash.hpp"

namespace nJinn {
	using namespace YAML;
	using namespace literals;

	void MaterialFamily::DescriptorAllocator::parseYAML(Node node)
	{
		vk::DescriptorSetLayoutCreateInfo descInfo;

		assert(node.IsSequence());
		uint32_t size = (uint32_t) node.size();

		std::unique_ptr<vk::DescriptorSetLayoutBinding[]> bindings(new vk::DescriptorSetLayoutBinding[size]);
		poolSizes = static_cast<std::unique_ptr<vk::DescriptorPoolSize[]>>(new vk::DescriptorPoolSize[size]);
		
		for(int i = 0; i < size; ++i){
			Node param = node[i];

			uint64_t typeId = hash(param["type"].as<std::string>());
			vk::DescriptorType type;

			switch (typeId) {
			case "texture"_hash: type = vk::DescriptorType::eSampledImage; break;
			case "uniforms"_hash: type = vk::DescriptorType::eUniformBufferDynamic; break;
			case "attachment"_hash: type = vk::DescriptorType::eInputAttachment; break;
			default: throw std::runtime_error("Unrecognized shader parameter type");
			}

			uint32_t count = param["count"].as<uint32_t>();

			bindings[i]
				.setBinding(i)
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				.setDescriptorType(type)
				.setDescriptorCount(count);
			poolSizes[i]
				.setType(type)
				.setDescriptorCount(descriptorPoolSize * count);
		}
		
		descInfo
			.setBindingCount(size)
			.setPBindings(bindings.get());

		mLayout = context->dev().createDescriptorSetLayout(descInfo);

		vk::DescriptorPoolCreateInfo mPoolCreateInfo;
		mPoolCreateInfo
			.setMaxSets(10)
			.setPoolSizeCount(size)
			.setPPoolSizes(poolSizes.get());

		vk::DescriptorPool pool = context->dev().createDescriptorPool(mPoolCreateInfo);
		mPools.push_front(pool);
	}

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

		vk::DescriptorSetLayout layouts[] = {
			mMaterialAllocator.mLayout,
			mObjectAllocator.mLayout
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
	{}

	MaterialFamily::~MaterialFamily()
	{
		context->dev().destroyPipelineLayout(mLayout);
	}

	void MaterialFamily::fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info)
	{
		// TODO read this from file
		static vk::PipelineRasterizationStateCreateInfo rasterinfo;
		rasterinfo.setLineWidth(1.0f);
		static vk::PipelineDepthStencilStateCreateInfo depthstencilInfo;
		depthstencilInfo
			.setDepthTestEnable(0)
			.setStencilTestEnable(0)
			.setMaxDepthBounds(1)
			.setDepthCompareOp(vk::CompareOp::eAlways);

		info
			.setLayout(mLayout)
			.setStageCount(mStageCount)
			.setPStages(mShaderStages)
			.setPColorBlendState(&mBlendState)
			.setPRasterizationState(&rasterinfo)
			.setPDepthStencilState(&depthstencilInfo);
	}

	MaterialFamily::DescriptorAllocator::~DescriptorAllocator() {
		context->dev().destroyDescriptorSetLayout(mLayout);
		for (auto it : mPools) {
			context->dev().destroyDescriptorPool(it);
		}
	}

	vk::DescriptorSet MaterialFamily::DescriptorAllocator::allocateDescriptorSet()
	{
		// TODO add support for freeing descriptor sets
		// TODO add synchronization
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
		mPools.push_front(pool);
		allocInfo.setDescriptorPool(pool);
		context->dev().allocateDescriptorSets(&allocInfo, &ret);
		return ret;
	}


	
}
