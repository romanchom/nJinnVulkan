#include "stdafx.hpp"
#include "SystemStartup.hpp"

#include "Config.hpp"

namespace nJinn {
	SystemStartup::SystemStartup() :
		mDebug(config.getValue<uint32_t>("debug")),
		mDebugBind(&mDebug, &nJinn::debug),
		mThreadPool(config.getValue<uint32_t>("threads")),
		mThreadPoolBind(&mThreadPool, &nJinn::threadPool),
		mContext(),
		mContextBind(&mContext, &nJinn::context),
		mScreen(config.getValue<uint32_t>("rendering.width"), config.getValue<uint32_t>("rendering.height")),
		mScreenBind(&mScreen, &nJinn::screen),
		mResourceUploader(),
		mResourceUploaderBind(&mResourceUploader, &nJinn::resourceUploader),
		mPipelineFactory(),
		mPipelineFactoryBind(&mPipelineFactory, &nJinn::pipelineFactory),
		mResourceManager(),
		mResourceManagerBind(&mResourceManager, &nJinn::resourceManager),
		mRendererSystem(),
		mRendererSystemBind(&mRendererSystem, &nJinn::rendererSystem),
		mClock(),
		mClockBind(&mClock, &nJinn::clock)
	{
	}

	SystemStartup::~SystemStartup()
	{
		context->dev().waitIdle();
	}
}