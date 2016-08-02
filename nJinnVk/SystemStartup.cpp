#include "stdafx.hpp"
#include "SystemStartup.hpp"

namespace nJinn {
	SystemStartup::SystemStartup() :
		mDebug(Debug::VerbosityLevel::all)
	{
		nJinn::debug = &mDebug;
		nJinn::rendererSystem = &mRendererSystem;
	}
}