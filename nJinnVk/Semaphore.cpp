#include "stdafx.hpp"
#include "Semaphore.hpp"

#include "Context.hpp"

namespace nJinn {
	Semaphore::Semaphore()
	{
		vk::SemaphoreCreateInfo info;
		semaphore = context->dev().createSemaphore(info);
	}

	Semaphore::~Semaphore()
	{
		context->dev().destroySemaphore(semaphore);
	}


}
