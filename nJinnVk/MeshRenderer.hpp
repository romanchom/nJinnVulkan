#pragma once

#include "Renderer.hpp"

#include "Mesh.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	class MeshRenderer : public Renderer {
	private:
		vk::Pipeline mPipeline;
		UniformBuffer mUniforms;
		vk::DescriptorSet mDescSet;
		vk::DescriptorPool mPool;
	protected:
		virtual void initialize() override;
		virtual void update() override;
	public:
		Mesh::p mMesh;
		virtual void draw(vk::CommandBuffer cmdbuf) override;
	};
}