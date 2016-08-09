#pragma once

#include "Renderer.hpp"

#include "Mesh.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	class MeshRenderer : public Renderer {
	protected:
		vk::Pipeline mPipeline;
		UniformBuffer mUniforms;
		Mesh::handle mMesh;
	public:
		void mesh(const Mesh::handle & mesh);
		virtual void update() override;
		virtual void draw(vk::CommandBuffer cmdbuf) override;
		virtual ~MeshRenderer();
		virtual bool validate() override;
	};
}