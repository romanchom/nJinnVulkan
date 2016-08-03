#pragma once

#include <vulkan.hpp>

#include "Mesh.hpp"
#include "MaterialFamily.hpp"
#include "CommandBuffer.hpp"

namespace nJinn {
	class RendererSystem {
	private:
		/*void createWorldDescriptorSet();
		void createObjectDescriptorSet();
		void createDrawDescriptorSet();
		void createLayout();
		void destroyLayout();*/
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
		class Screen * screen;
	public:
		RendererSystem();
		~RendererSystem();

		void update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsCw);

		vk::Sampler immutableSamplers[immutableSamplerCount];

		CommandBuffer cmdbuf;

		void setScreen(class Screen * scr) { screen = scr; }
	};

	extern RendererSystem * rendererSystem;
}