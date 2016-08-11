#pragma once

#include <list>
#include <memory>
#include <vulkan.hpp>
#include <yaml-cpp/yaml.h>

#include "DescriptorSet.hpp"

namespace nJinn {
	class DescriptorAllocator {
	private:
		enum shaderTypes {
			descriptorPoolSize = 10,
		};

		std::list<vk::DescriptorPool> mPools;
		std::unique_ptr<vk::DescriptorPoolSize[]> poolSizes;
		vk::DescriptorPoolCreateInfo mPoolCreateInfo;
		vk::DescriptorSetLayout mLayout;
		uint32_t mBindingCount;
		uint32_t mDescriptorCount;
	public:
		DescriptorAllocator();
		~DescriptorAllocator();
		void allocateDescriptorSet(class DescriptorSet & set);
		void parseYAML(YAML::Node node);
		vk::DescriptorSetLayout layout() { return mLayout; }

		friend class DescriptorSet;
	};
}