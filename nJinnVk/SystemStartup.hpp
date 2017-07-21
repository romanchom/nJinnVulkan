#pragma once

#include "PointerBind.hpp"
#include "Debug.hpp"
#include "ThreadPool.hpp"
#include "Context.hpp"
#include "Memory.hpp"
#include "Screen.hpp"
#include "ResourceUploader.hpp"
#include "PipelineFactory.hpp"
#include "ResourceManager.hpp"
#include "RendererSystem.hpp"
#include "Clock.hpp"

namespace nJinn {
	class SystemStartup
	{
	public:
		SystemStartup();
		~SystemStartup();
	private:
		Debug mDebug;
		PointerBind<Debug> mDebugBind;

		ThreadPool mThreadPool;
		PointerBind<ThreadPool> mThreadPoolBind;
		
		Context mContext;
		PointerBind<Context> mContextBind;

		Memory mMemory;
		PointerBind<Memory> mMemoryBind;

		Screen mScreen;
		PointerBind<Screen> mScreenBind;

		ResourceUploader mResourceUploader;
		PointerBind<ResourceUploader> mResourceUploaderBind;
		
		PipelineFactory mPipelineFactory;
		PointerBind<PipelineFactory> mPipelineFactoryBind;

		ResourceManager mResourceManager;
		PointerBind<ResourceManager> mResourceManagerBind;
		
		RendererSystem mRendererSystem;
		PointerBind<RendererSystem> mRendererSystemBind;

		Clock mClock;
		PointerBind<Clock> mClockBind;
	};
}