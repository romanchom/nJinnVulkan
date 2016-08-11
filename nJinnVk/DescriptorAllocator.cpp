#include "stdafx.hpp"
#include "DescriptorAllocator.hpp"

#include "Context.hpp"
#include "Hash.hpp"

namespace nJinn {
	using namespace YAML;

	DescriptorAllocator::DescriptorAllocator() :
		mBindingCount(0),
		mDescriptorCount(0)
	{}

	DescriptorAllocator::~DescriptorAllocator() {
		context->dev().destroyDescriptorSetLayout(mLayout);
		for (auto it : mPools) {
			context->dev().destroyDescriptorPool(it);
		}
	}

	void DescriptorAllocator::allocateDescriptorSet(class DescriptorSet & set)
	{
		// TODO add synchronization
		vk::DescriptorSet descriptorSet = nullptr;
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo
			.setDescriptorSetCount(1)
			.setPSetLayouts(&mLayout);
		set.mBindingCount = mBindingCount;
		set.mDescriptorCount = mDescriptorCount;

		uint32_t  tryCount = (uint32_t)mPools.size();
		while (tryCount > 0) {
			auto it = mPools.begin();
			allocInfo.setDescriptorPool(*it);
			if (vk::Result::eSuccess == context->dev().allocateDescriptorSets(&allocInfo, &descriptorSet)) {
				set.mDescriptorSet = descriptorSet;
				set.mParentPool = *it;
			}
			else {
				// move full pool to the back
				mPools.splice(mPools.end(), mPools, it);
				--tryCount;
			}
		}
		vk::DescriptorPool pool = context->dev().createDescriptorPool(mPoolCreateInfo);
		mPools.push_front(pool);
		allocInfo.setDescriptorPool(pool);
		context->dev().allocateDescriptorSets(&allocInfo, &descriptorSet);
		set.mDescriptorSet = descriptorSet;
		set.mParentPool = pool;
	}

	void DescriptorAllocator::parseYAML(Node node)
	{
		vk::DescriptorSetLayoutCreateInfo descInfo;

		assert(node.IsSequence());
		mBindingCount = (uint32_t)node.size();

		std::unique_ptr<vk::DescriptorSetLayoutBinding[]> bindings(new vk::DescriptorSetLayoutBinding[mBindingCount]);
		poolSizes = static_cast<std::unique_ptr<vk::DescriptorPoolSize[]>>(new vk::DescriptorPoolSize[mBindingCount]);
		

		for (int i = 0; i < mBindingCount; ++i) {
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
			mDescriptorCount += count;

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
			.setBindingCount(mBindingCount)
			.setPBindings(bindings.get());

		mLayout = context->dev().createDescriptorSetLayout(descInfo);

		mPoolCreateInfo
			.setMaxSets(descriptorPoolSize)
			.setPoolSizeCount(mBindingCount)
			.setPPoolSizes(poolSizes.get());


		// TODO solve why creating descriptor pool here makes whole vulkan go nuts
		/*vk::DescriptorPool pool = context->dev().createDescriptorPool(mPoolCreateInfo);
		mPools.push_front(pool);*/
	}

}