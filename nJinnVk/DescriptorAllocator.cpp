#include "stdafx.hpp"
#include "DescriptorAllocator.hpp"

#include "Context.hpp"
#include "Hash.hpp"

namespace nJinn {
	using namespace YAML;
	using namespace literals;

	DescriptorAllocator::DescriptorAllocator(uint32_t poolSize) :
		mBindingCount(0),
		mDescriptorCount(0),
		mPoolSize(poolSize)
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
				return;
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

		assert(node.IsSequence());
		auto bindingCount = static_cast<uint32_t>(node.size());

		auto bindings = std::make_unique<vk::DescriptorSetLayoutBinding[]>(bindingCount);
		
		for (int i = 0; i < bindingCount; ++i) {
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
		}

		initialize(bindings.get(), bindingCount);
	}

	void DescriptorAllocator::initialize(vk::DescriptorSetLayoutBinding * bindings, uint32_t count)
	{
		mBindingCount = count;

		vk::DescriptorSetLayoutCreateInfo descInfo;
		descInfo
			.setBindingCount(mBindingCount)
			.setPBindings(bindings);

		mLayout = context->dev().createDescriptorSetLayout(descInfo);

		mPoolSizes = std::make_unique<vk::DescriptorPoolSize[]>(count);
		for (uint32_t i = 0; i < count; ++i) {
			mPoolSizes[i]
				.setType(bindings[i].descriptorType)
				.setDescriptorCount(bindings[i].descriptorCount * mPoolSize);

			mDescriptorCount += bindings[i].descriptorCount;
		}

		mPoolCreateInfo
			.setMaxSets(mPoolSize)
			.setPoolSizeCount(mBindingCount)
			.setPPoolSizes(mPoolSizes.get())
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		vk::DescriptorPool pool = context->dev().createDescriptorPool(mPoolCreateInfo);
		mPools.push_front(pool);
	}

}