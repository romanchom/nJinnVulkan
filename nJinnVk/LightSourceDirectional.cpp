#include "stdafx.hpp"
#include "LightSourceDirectional.hpp"

#include "ResourceManager.hpp"
#include "PipelineFactory.hpp"
#include "RendererSystem.hpp"

namespace nJinn {
	LightSourceDirectional::LightSourceDirectional()
	{
		mMaterial = resourceManager->get<MaterialFamily>("quad.yml", ResourceLoadPolicy::Immediate);
		mLightVolume = resourceManager->get<Mesh>("quad.vbm", ResourceLoadPolicy::Immediate);
		
		//mMaterial->mObjectAllocator.allocateDescriptorSet(mDescriptorSet);
		mPipeline = pipelineFactory->createPipeline(*mMaterial, *mLightVolume,
			rendererSystem->mDeferredRenderPass, rendererSystem->lightingSubpassIndex);

	}

	LightSourceDirectional::~LightSourceDirectional()
	{
	}

	void LightSourceDirectional::draw(vk::CommandBuffer cmdbuf)
	{
		cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		mLightVolume->bind(cmdbuf);
		mLightVolume->draw(cmdbuf);
	}
}

