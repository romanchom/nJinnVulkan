#pragma once

#include <vulkan.hpp>

#include "Component.hpp"

#include "MaterialFamily.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "DescriptorSet.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	class LightSource : public Component
	{
	protected:
		MaterialFamily::handle mMaterial;
		vk::Pipeline mPipeline;
		Mesh::handle mLightVolume;
		//DescriptorSet mDescriptorSet;
		//Material mMaterialInstance;
		//UniformBuffer mUniforms;
	public:
		//virtual void update();
		virtual void draw(vk::CommandBuffer cmdbuf) = 0;
		LightSource();
		virtual ~LightSource();
	};
}
