#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class Semaphore {
	private:
		vk::Semaphore semaphore;
		Semaphore(const Semaphore &) = delete;
		Semaphore & operator=(const Semaphore &) = delete;
	public:
		Semaphore();
		~Semaphore();

		operator vk::Semaphore() { return semaphore; }

		vk::Semaphore * get() { return &semaphore; }
	};
}