#pragma once

#include "Renderer.hpp"

#include "Mesh.hpp"

namespace nJinn {
	class MeshRenderer : public Renderer {
	private:
		vk::Pipeline mPipeline;
	protected:
		virtual void initialize() override;
	public:
		Mesh::p mMesh;
		virtual void draw(vk::CommandBuffer cmdbuf) override;
	};
}