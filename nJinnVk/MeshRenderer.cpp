#include "stdafx.hpp"
#include "MeshRenderer.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Material.hpp"
#include "MaterialFamily.hpp"
#include "Screen.hpp"
#include <chrono>

namespace nJinn {
	void MeshRenderer::initialize()
	{
		vk::PipelineRasterizationStateCreateInfo rasterinfo;
		rasterinfo.setLineWidth(1.0f);
		vk::PipelineDepthStencilStateCreateInfo depthstencilInfo;
		depthstencilInfo
			.setDepthTestEnable(0)
			.setStencilTestEnable(0)
			.setMaxDepthBounds(1)
			.setDepthCompareOp(vk::CompareOp::eAlways);
		mPipeline = Context::pipeFact().createPipeline(*mForwardMaterial->family(), *mMesh, Application::screen->renderPass(), 0, &rasterinfo, &depthstencilInfo);


		mDescSet = mForwardMaterial->family()->mObjectAllocator.allocateDescriptorSet();

		mUniforms.initialize(16);

		vk::DescriptorBufferInfo bufferInfo;
		mUniforms.fillDescriptorInfo(bufferInfo);

		vk::WriteDescriptorSet descWrite;
		descWrite
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDstArrayElement(0)
			.setDstBinding(0)
			.setDstSet(mDescSet)
			.setPBufferInfo(&bufferInfo);

		Context::dev().updateDescriptorSets(1, &descWrite, 0, nullptr);
	}

	struct uniforms {
		float x, y, z, w;
	};

	void MeshRenderer::update()
	{
		uniforms * uni = mUniforms.acquire<uniforms>();
		std::chrono::duration<double> a = std::chrono::high_resolution_clock::now().time_since_epoch();
		double s = sin(a.count());
		double c = cos(a.count());
		uni->x = c * 0.5;
		uni->y = s * 0.5;
		uni->z = 0.2f;
		uni->w = 0.2f;
	}

	void MeshRenderer::draw(vk::CommandBuffer cmdbuf)
	{
		cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		mForwardMaterial->bind(cmdbuf);
		uint32_t offset = mUniforms.offset();
		cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mForwardMaterial->family()->layout(), 1, 1, &mDescSet, 1, &offset);
		// bind renderer descriptor sets
		mMesh->bind(cmdbuf);
		mMesh->draw(cmdbuf);
	}
}
