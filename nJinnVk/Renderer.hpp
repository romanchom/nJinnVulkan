#pragma once

#include <unordered_set>
#include <vulkan.hpp>
#include "Component.hpp"

#include "Material.hpp"

namespace nJinn {
	class Renderer : public Component<10000> {
	public:
		static std::unordered_set<Renderer *> sRenderers;

		Material * mDepthMaterial;
		Material * mDeferredMaterial;
		Material * mForwardMaterial;
		vk::DescriptorSet mDescriptorSet; // object uniforms
		
		virtual void draw(vk::CommandBuffer cmdbuf) = 0;
		
		Renderer();
		virtual ~Renderer() override;
	};
}