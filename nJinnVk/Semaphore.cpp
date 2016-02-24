#include "stdafx.hpp"
#include "Semaphore.hpp"

#include "Context.hpp"

namespace nJinn {
	Semaphore::Semaphore()
	{
		vk::SemaphoreCreateInfo info;
		dc(vk::createSemaphore(Context::dev(), &info, nullptr, &semaphore));
	}

	Semaphore::~Semaphore()
	{
		vk::destroySemaphore(Context::dev(), semaphore, nullptr);
	}


}
