#include "stdafx.hpp"
#include "Debug.hpp"

namespace nJinn {
	Debug * debug = nullptr;

	Debug::Debug(int initialVerbosity) :
		logFileStream("nJinnLog"),
		outputConsole(),
		textOutput(logFileStream, std::cout),
		maximumVerbosityLevel((int) initialVerbosity)
	{}

	void Debug::setVerbosity(int level)
	{
		maximumVerbosityLevel = (int)level;
	}
}
