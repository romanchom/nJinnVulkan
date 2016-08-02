#include "stdafx.hpp"
#include "Debug.hpp"

namespace nJinn {
	Debug * debug = nullptr;

	Debug::Debug(VerbosityLevel initialVerbosity) :
		logFileStream("nJinnLog"),
		outputConsole(),
		textOutput(logFileStream, std::cout),
		maximumVerbosityLevel((int) initialVerbosity)
	{}

	void Debug::setVerbosity(VerbosityLevel level)
	{
		maximumVerbosityLevel = (int)level;
	}
}
