#include "stdafx.hpp"
#include "Config.hpp"

#include <fstream>

namespace nJinn {
	using namespace boost::program_options;

	Config::Config()
	{
		optionsDescriptions.add_options()
			("threads", value<uint32_t>()->default_value(1), "thread pool threads")
			("shadowMapRes", value<uint32_t>()->default_value(256), "shadow map resolution")
			("debug", value<uint32_t>()->default_value(3), "logging verbosity (0 - 3)")
			("debugVK", value<uint32_t>()->default_value(1), "Vulkan logging verbosity (0 - 5)")
			
			("swapchain.backBufferCount", value<uint32_t>()->default_value(2), "number of backBuffers")
			("swapchain.maxQueuedFrames", value<uint32_t>()->default_value(2), "maxiumum number of simultaneously queued frames")
			
			("rendering.width", value<uint32_t>()->default_value(640), "screen width")
			("rendering.height", value<uint32_t>()->default_value(480), "screen height");
	}

	boost::program_options::options_description_easy_init Config::addOptions()
	{
		return optionsDescriptions.add_options();
	}

	void Config::parseDefaultConfigFile()
	{
		parseConfigFile("config.cfg");
	}

	void Config::parseCommandLine(const wchar_t * line)
	{
		auto arguments = split_winmain(line);
		store(wcommand_line_parser(arguments).options(optionsDescriptions).run(), variables);
	}

	void Config::parseCommandLine(int argc, const char ** argv)
	{
		store(command_line_parser(argc, argv).options(optionsDescriptions).run(), variables);
	}

	void Config::parseConfigFile(const std::string & fileName)
	{
		std::ifstream configFile(fileName);
		if (configFile.good()) {
			auto options = parse_config_file(configFile, optionsDescriptions, true);
			// trick boost into believing our options are genuine
			// TODO properly handle different type of data
			for (auto & option : options.options) {
				if (option.unregistered) {
					option.unregistered = false;
					optionsDescriptions.add_options()(option.string_key.c_str(), value<std::string>());
				}
			}

			store(options, variables);
		}
	}

	Config config;
}