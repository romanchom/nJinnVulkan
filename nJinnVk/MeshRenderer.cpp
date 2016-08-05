#include "stdafx.hpp"
#include "MeshRenderer.hpp"

#include <chrono>
#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Material.hpp"
#include "MaterialFamily.hpp"
#include "Screen.hpp"
#include "ResourceManager.hpp"
#include "RendererSystem.hpp"

namespace nJinn {
	struct uniforms {
		float x, y, z, w;
	};

	void MeshRenderer::mesh(const Mesh::handle & mesh)
	{
		mMesh = mesh;
		resourceManager->onResourceLoaded(mesh, [=]{ return validate(); });
	}

	void MeshRenderer::update()
	{
		uniforms * uni = mUniforms.acquire<uniforms>();
		std::chrono::duration<double> a = std::chrono::high_resolution_clock::now().time_since_epoch();
		double s = sin(a.count());
		double c = cos(a.count());
		uni->x = (float) (c * 0.5);
		uni->y = (float) (s * 0.5);
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

	MeshRenderer::~MeshRenderer()
	{
		context->dev().destroyPipeline(mPipeline);
	}

	bool nJinn::MeshRenderer::validate()
	{
		if (mMesh != nullptr && mMesh->isLoaded() && Renderer::isValid()) {
			mForwardMaterial = static_cast<std::unique_ptr<Material>>(mMaterialFamily->instantiate());
			vk::PipelineRasterizationStateCreateInfo rasterinfo;
			rasterinfo.setLineWidth(1.0f);
			vk::PipelineDepthStencilStateCreateInfo depthstencilInfo;
			depthstencilInfo
				.setDepthTestEnable(0)
				.setStencilTestEnable(0)
				.setMaxDepthBounds(1)
				.setDepthCompareOp(vk::CompareOp::eAlways);
			mPipeline = pipelineFactory->createPipeline(*mForwardMaterial->family(), *mMesh, Application::screen->renderPass(), 0, &rasterinfo, &depthstencilInfo);
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

			context->dev().updateDescriptorSets(1, &descWrite, 0, nullptr);

			rendererSystem->registerRenderer(this);
			return true;
		}
		else {
			return false;
		}
	}
}
