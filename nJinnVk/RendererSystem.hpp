#pragma once

#include <vulkan.hpp>

#include "Mesh.hpp"
#include "Material.hpp"

namespace nJinn {
	class RendererSystem {
	private:
		void createWorldDescriptorSet();
		void createObjectDescriptorSet();
		void createDrawDescriptorSet();
		void createLayout();
		void destroyLayout();
		enum {
			worldDescriptorSetIndex = 0,
			objectDescriptorSetIndex,
			drawDescriptorSetIndex,
			descriptorSetCount,
			immutableSamplerCount = 2,
			worldDescriptorSetBindingCount = 2,
			objectDescriptorSetBindingCount = 4,
			drawDescriptorSetBindingCount = 1
		};
	public:
		RendererSystem();
		~RendererSystem();

		void update(vk::CommandBuffer cmdBuf);

		vk::DescriptorSetLayout descriptorSetLayouts[descriptorSetCount];
		vk::PipelineLayout pipelineLayout;
		vk::Sampler immutableSamplers[immutableSamplerCount];


		Mesh::p someMesh;
		Material someMaterial;
		vk::Pipeline somePipe;
	};
}