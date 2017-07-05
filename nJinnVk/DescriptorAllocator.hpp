#pragma once

#include <list>
#include <memory>
#include <vulkan.hpp>
#include <yaml-cpp/yaml.h>

#include "DescriptorSet.hpp"

namespace nJinn {
	class DescriptorAllocator {
	private:
		std::list<vk::DescriptorPool> mPools;
		std::unique_ptr<vk::DescriptorPoolSize[]> mPoolSizes;
		vk::DescriptorPoolCreateInfo mPoolCreateInfo;
		vk::DescriptorSetLayout mLayout;
		uint32_t mBindingCount;
		uint32_t mDescriptorCount;
		uint32_t mPoolSize;
	public:
		DescriptorAllocator(uint32_t poolSize = 16);
		~DescriptorAllocator();
		void allocateDescriptorSet(class DescriptorSet & set);
		void parseYAML(YAML::Node node);
		void initialize(vk::DescriptorSetLayoutBinding * bindings, uint32_t count);
		vk::DescriptorSetLayout layout() { return mLayout; }

		friend class DescriptorSet;
	};
}