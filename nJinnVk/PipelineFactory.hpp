#pragma once

#include <vector>
#include <vulkan.hpp>
#include "Material.hpp"
#include "Mesh.hpp"

namespace nJinn {
	class PipelineFactory {
	private:
		vk::PipelineCache cache;
	public:
		PipelineFactory();
		~PipelineFactory();

		vk::Pipeline createPipeline(Material & material, Mesh & mesh, vk::PipelineLayout layout, vk::RenderPass pass, uint32_t subpass,
			vk::PipelineRasterizationStateCreateInfo * rasterInfo, vk::PipelineDepthStencilStateCreateInfo * depthStencilInfo);
	};
}