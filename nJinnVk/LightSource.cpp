#include "stdafx.hpp"
#include "LightSource.hpp"

#include "RendererSystem.hpp"

namespace nJinn
{
	LightSource::LightSource() :
		mMaterial(nullptr),
		mPipeline(nullptr),
		mLightVolume(nullptr)
	{
		rendererSystem->registerLightSource(this);
	}

	LightSource::~LightSource(){
		rendererSystem->unregisterLightSource(this);
		context->dev().destroyPipeline(mPipeline);
	}

}
