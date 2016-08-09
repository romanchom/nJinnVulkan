#pragma once

#include <vector>
#include <vulkan.hpp>
#include "MaterialFamily.hpp"
#include "Mesh.hpp"

namespace nJinn {
	class PipelineFactory {
	private:
		vk::PipelineCache cache;
	public:
		PipelineFactory();
		~PipelineFactory();

		vk::Pipeline createPipeline(MaterialFamily & material, Mesh & mesh, vk::RenderPass pass, uint32_t subpass);
	};

	extern PipelineFactory * pipelineFactory;
}