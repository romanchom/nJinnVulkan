#include "stdafx.hpp"
#include "Semaphore.hpp"

#include "Context.hpp"

namespace nJinn {
	Semaphore::Semaphore()
	{
		vk::SemaphoreCreateInfo info;
		semaphore = Context::dev().createSemaphore(info);
	}

	Semaphore::~Semaphore()
	{
		Context::dev().destroySemaphore(semaphore);
	}


}
