#pragma once

#include <list>
#include <vulkan.hpp>

#include "Resource.hpp"
#include "Shader.hpp"

namespace nJinn {
	class MaterialFamily : public Resource {
		class DescriptorAllocator {
		private:
		public:
			~DescriptorAllocator();
			std::list<vk::DescriptorPool> mPools;
			vk::DescriptorPoolCreateInfo mPoolCreateInfo;
			vk::DescriptorSetLayout mLayout;
			vk::DescriptorSet allocateDescriptorSet();
		};
		
		Shader::handle vertexShader;
		Shader::handle fragmentShader;

		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo blendState;
		vk::PipelineColorBlendAttachmentState state;
		uint32_t mStageCount;
		vk::PipelineShaderStageCreateInfo stages[2];

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