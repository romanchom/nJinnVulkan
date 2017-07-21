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

namespace nJinn {
	void nJinnStart() {
		debug = new Debug(config.getValue<uint32_t>("debug"));
		threadPool = new ThreadPool(config.getValue<uint32_t>("threads"));
		context = new Context();
		memory = new Memory();
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
		UniformBuffer::collect();
		safeDelete(memory);
		safeDelete(context);
		safeDelete(threadPool);
		safeDelete(debug);
	}
}