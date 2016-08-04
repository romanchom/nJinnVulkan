#pragma once

#include <list>
#include <vulkan.hpp>

#include "Resource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class MaterialFamily : public Resource {
		class DescriptorAllocator {
		public:
			~DescriptorAllocator();
			std::list<vk::DescriptorPool> mPools;
			vk::DescriptorPoolCreateInfo mPoolCreateInfo;
			vk::DescriptorSetLayout mLayout;
			vk::DescriptorSet allocateDescriptorSet();
		};
		
		enum shaderTypes {
			shaderCount = 5, 
		};

		Shader::handle mShaders[shaderCount];
		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo mBlendState;
		vk::PipelineColorBlendAttachmentState mBlendAttachmentState;
		uint32_t mStageCount;
		vk::PipelineShaderStageCreateInfo mShaderStages[shaderCount];

		vk::DescriptorPoolSize mPoolSizes[3];

		DescriptorAllocator mMaterialAllocator;
	public:
		typedef std::shared_ptr<MaterialFamily> handle;
		DescriptorAllocator mObjectAllocator;
		
		MaterialFamily();
		~MaterialFamily();
		virtual void load(const std::string & name) override;
		class Material * instantiate();
		void fillPipelineInfo(vk::GraphicsPipelineCreateInfo & info);
		vk::PipelineLayout layout() { return mLayout; }
	};
}