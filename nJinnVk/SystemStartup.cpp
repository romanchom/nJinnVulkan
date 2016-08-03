#include "stdafx.hpp"
#include "SystemStartup.hpp"

#include "Config.hpp"

namespace nJinn {
	SystemStartup::SystemStartup() :
		mDebug(config.getValue<uint32_t>("debug")),
		mDebugBind(&mDebug, &nJinn::debug),
		mContext(),
		mContextBind(&mContext, &nJinn::context),
		mResourceUploader(),
		mResourceUploaderBind(&mResourceUploader, &nJinn::resourceUploader),
		mPipelineFactory(),
		mPipelineFactoryBind(&mPipelineFactory, &nJinn::pipelineFactory),
		mRendererSystem(),
		mRendererSystemBind(&mRendererSystem, &nJinn::rendererSystem)
	{
	}

	SystemStartup::~SystemStartup()
	{
		context->dev().waitIdle();
	}
}