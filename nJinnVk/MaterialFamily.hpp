#pragma once

#include <list>
#include <vulkan.hpp>
#include <yaml-cpp/yaml.h>

#include "Resource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class MaterialFamily : public Resource {
		class DescriptorAllocator {
		public:
			~DescriptorAllocator();
			std::list<vk::DescriptorPool> mPools;
			std::unique_ptr<vk::DescriptorPoolSize[]> poolSizes;

			vk::DescriptorPoolCreateInfo mPoolCreateInfo;
			vk::DescriptorSetLayout mLayout;
			vk::DescriptorSet allocateDescriptorSet();
			void parseYAML(YAML::Node node);
		};
		
		enum shaderTypes {
			shaderCount = 5, 
			descriptorPoolSize = 10,
		};

		Shader::handle mShaders[shaderCount];
		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo mBlendState;
		vk::PipelineColorBlendAttachmentState mBlendAttachmentState[2];
		uint32_t mStageCount;
		vk::PipelineShaderStageCreateInfo mShaderStages[shaderCount];


		DescriptorAllocator mMaterialAllocator;
	public:
		typedef std::shared_ptr<MaterialFamily> handle;
		DescriptorAllocator mObjectAllocator;
		
		MaterialFamily();
		~MaterialFamily();
		virtual void load(const std::string & name) override;
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		vk::PipelineLayout layout() { return mLayout; }
	};
}