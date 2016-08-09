#pragma once
#include "Component.hpp"

#include <unordered_set>
#include <memory>
#include <vulkan.hpp>

#include "MaterialFamily.hpp"
#include "Material.hpp"

namespace nJinn {
	class Renderer : public Component {
	protected:
		MaterialFamily::handle mMaterialFamily;
		//std::unique_ptr<Material> mForwardMaterial;
		vk::DescriptorSet mDescSet; // object uniforms

		virtual void update() = 0;
		virtual void draw(vk::CommandBuffer cmdbuf) = 0;
		virtual bool validate();
		bool isValid() { return mMaterialFamily != nullptr && mMaterialFamily->isLoaded(); }
	public:
		Renderer();
		virtual ~Renderer() override;
		void materialFamily(const MaterialFamily::handle & material);

		friend class RendererSystem;
	};
}