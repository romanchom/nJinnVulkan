#pragma once

#include <list>
#include <vulkan.hpp>
#include <yaml-cpp/yaml.h>

#include "Resource.hpp"
#include "Shader.hpp"
#include "DescriptorAllocator.hpp"

namespace nJinn {
	class MaterialFamily : public Resource {
		enum shaderTypes {
			shaderCount = 5, 
		};

		Shader::handle mShaders[shaderCount];
		vk::PipelineLayout mLayout;
		vk::PipelineColorBlendStateCreateInfo mBlendState;
		vk::PipelineColorBlendAttachmentState mBlendAttachmentState[2];
		vk::PipelineDepthStencilStateCreateInfo mDepthStencilInfo;
		uint32_t mStageCount;
		vk::PipelineShaderStageCreateInfo mShaderStages[shaderCount];
		vk::PipelineRasterizationStateCreateInfo mRasterisationInfo;

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