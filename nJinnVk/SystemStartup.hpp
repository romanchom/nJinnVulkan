#pragma once

#include "Debug.hpp"
#include "RendererSystem.hpp"
#include "Context.hpp"


namespace nJinn {
	struct SystemStartup
	{
		SystemStartup();

		Debug mDebug;
		Context mContext;
		RendererSystem mRendererSystem;
	};
}