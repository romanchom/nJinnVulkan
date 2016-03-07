#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class Material {
	private:
		vk::DescriptorSet mDescriptorSet; // Material textures and uniforms, colors
		class MaterialFamily * mMaterialFamily;

		friend class MaterialFamily;
	public:
		class MaterialFamily * family() { return mMaterialFamily; }
		void bind(vk::CommandBuffer cmdbuf);
	};
}
