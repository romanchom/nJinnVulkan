#pragma once

#include "PointerBind.hpp"
#include "Debug.hpp"
#include "Context.hpp"
#include "ResourceUploader.hpp"
#include "RendererSystem.hpp"
#include "PipelineFactory.hpp"


namespace nJinn {
	class SystemStartup
	{
	public:
		SystemStartup();
		~SystemStartup();
	private:
		Debug mDebug;
		PointerBind<Debug> mDebugBind;
		
		Context mContext;
		PointerBind<Context> mContextBind;

		ResourceUploader mResourceUploader;
		PointerBind<ResourceUploader> mResourceUploaderBind;
		
		PipelineFactory mPipelineFactory;
		PointerBind<PipelineFactory> mPipelineFactoryBind;
		
		RendererSystem mRendererSystem;
		PointerBind<RendererSystem> mRendererSystemBind;
	};
}