#include "stdafx.hpp"
#include "MeshRenderer.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Material.hpp"
#include "MaterialFamily.hpp"
#include "Screen.hpp"

namespace nJinn {
	void MeshRenderer::initialize()
	{
		vk::PipelineRasterizationStateCreateInfo rasterinfo;
		vk::PipelineDepthStencilStateCreateInfo depthstencilInfo;
		depthstencilInfo
			.depthTestEnable(0)
			.stencilTestEnable(0)
			.maxDepthBounds(1)
			.depthCompareOp(vk::CompareOp::eAlways);
		mPipeline = Context::pipeFact().createPipeline(*mForwardMaterial->family(), *mMesh, Application::screen()->renderPass(), 0, &rasterinfo, &depthstencilInfo);
	}

	void MeshRenderer::draw(vk::CommandBuffer cmdbuf)
	{
		vk::cmdBindPipeline(cmdbuf, vk::PipelineBindPoint::eGraphics, mPipeline);
		mForwardMaterial->bind(cmdbuf);
		// bind renderer descriptor sets
		mMesh->bind(cmdbuf);
		mMesh->draw(cmdbuf);
	}
}
