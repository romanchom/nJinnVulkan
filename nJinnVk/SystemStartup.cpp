#include "stdafx.hpp"
#include "SystemStartup.hpp"

#include "Config.hpp"

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
#include "UniformBuffer.hpp"

namespace nJinn {
	using namespace nJinn::literals;
	void nJinnStart() {
		debug = new Debug(config.getValue<uint32_t>("debug"));
		threadPool = new ThreadPool(config.getValue<uint32_t>("threads"));
		context = new Context();
		memory = new Memory();
		uniformManager = new UniformManager(1_MiB);
		screen = new Screen(config.getValue<uint32_t>("rendering.width"), config.getValue<uint32_t>("rendering.height"));
		resourceUploader = new ResourceUploader();
		pipelineFactory = new PipelineFactory();
		resourceManager = new ResourceManager();
		rendererSystem = new RendererSystem();
		clock = new Clock();
	}

	template<typename T>
	void safeDelete(T *& pointer) {
		delete pointer;
		pointer = nullptr;
	}

	void nJinnStop() {
		context->dev().waitIdle();
		safeDelete(clock);
		safeDelete(rendererSystem);
		safeDelete(resourceManager);
		safeDelete(pipelineFactory);
		safeDelete(resourceUploader);
		safeDelete(screen);
		safeDelete(uniformManager);
		safeDelete(memory);
		safeDelete(context);
		safeDelete(threadPool);
		safeDelete(debug);
	}
}