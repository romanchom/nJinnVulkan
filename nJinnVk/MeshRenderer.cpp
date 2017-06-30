#include "stdafx.hpp"
#include "MeshRenderer.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Material.hpp"
#include "MaterialFamily.hpp"
#include "Screen.hpp"
#include "ResourceManager.hpp"
#include "RendererSystem.hpp"
#include "Clock.hpp"
#include "GameObject.hpp"

namespace nJinn {
	namespace detail {
		struct ObjectUniforms {
			Eigen::Matrix4f model;
		};
	}

	void MeshRenderer::mesh(const Mesh::handle & mesh)
	{
		mMesh = mesh;
		resourceManager->onResourceLoaded(mesh, [=]{ return validate(); });
	}

	void MeshRenderer::update()
	{
		auto uniforms = mUniforms.acquire<detail::ObjectUniforms>();
		uniforms->model = owner()->transform().cast<float>();
	}

	void MeshRenderer::draw(vk::CommandBuffer cmdbuf)
	{
		cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		uint32_t offset = mUniforms.offset();
		cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mMaterialFamily->layout(), 1, 1, mDescSet.get(), 1, &offset);
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
		bool allOk = (mMesh != nullptr && mMesh->isLoaded() && Renderer::isValid());
		if(allOk){
			mPipeline = pipelineFactory->createPipeline(*mMaterialFamily, *mMesh, rendererSystem->renderPass(), rendererSystem->geometrySubpassIndex);
			mMaterialFamily->mObjectAllocator.allocateDescriptorSet(mDescSet);

			mUniforms.initialize<detail::ObjectUniforms>();

			mDescSet.write()
				.uniformBuffer(&mUniforms, 0);

			rendererSystem->registerRenderer(this);
		}
		return allOk;
	}
}
