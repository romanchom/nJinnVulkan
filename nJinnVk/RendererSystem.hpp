#pragma once

#include <boost/unordered_set.hpp>
#include <vulkan.hpp>

#include "Mesh.hpp"
#include "MaterialFamily.hpp"
#include "CommandBuffer.hpp"

namespace nJinn {
	class RendererSystem {
	public:
		typedef boost::unordered_set<class Renderer *> set_t;
	private:
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

		set_t mRenderersSet;

		class Screen * screen;
	public:
		RendererSystem();
		~RendererSystem();

		void registerRenderer(class Renderer * renderer) { mRenderersSet.emplace(renderer); }
		void unregisterRenderer(class Renderer * renderer) { mRenderersSet.erase(renderer); }

		void update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsCw);

		vk::Sampler immutableSamplers[immutableSamplerCount];

		CommandBuffer cmdbuf;

		void setScreen(class Screen * scr) { screen = scr; }
	};

	extern RendererSystem * rendererSystem;
}