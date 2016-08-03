#pragma once

#include <list>
#include <vulkan.hpp>

#include "TrackedResource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class MaterialFamily : public TrackedResource<MaterialFamily> {
		class DescriptorAllocator {
		private:
		public:
			~DescriptorAllocator();
			std::list<vk::DescriptorPool> mPools;
			vk::DescriptorPoolCreateInfo mPoolCreateInfo;
			vk::DescriptorSetLayout mLayout;
			vk::DescriptorSet allocateDescriptorSet();
		};
		
		Shader::p vertexShader;
		Shader::p fragmentShader;

		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo blendState;
		vk::PipelineColorBlendAttachmentState state;
		uint32_t mStageCount;
		vk::PipelineShaderStageCreateInfo stages[2];

		vk::DescriptorPoolSize mPoolSizes[3];

		DescriptorAllocator mMaterialAllocator;
	public:
		DescriptorAllocator mObjectAllocator;
		MaterialFamily(const std::string & name);
		~MaterialFamily();
		class Material * instantiate();
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		vk::PipelineLayout layout() { return mLayout; }
	};
}