#pragma once

#include <vulkan.hpp>

#include "Context.hpp"

namespace nJinn {
	class Semaphore {
	private:
		vk::Semaphore mSemaphore;
		Semaphore(const Semaphore &) = delete;
		Semaphore & operator=(const Semaphore &) = delete;
		void destroy() {
			if (nullptr != mSemaphore)
				context->dev().destroySemaphore(mSemaphore);
		}
	public:
		Semaphore() {
			vk::SemaphoreCreateInfo info;
			mSemaphore = context->dev().createSemaphore(info);
		}
		~Semaphore() {
			destroy();
		}
		Semaphore(Semaphore && orig) :
			mSemaphore(orig.mSemaphore)
		{
			orig.mSemaphore = nullptr;
		}
		Semaphore & operator=(Semaphore && orig) {
			destroy();
			mSemaphore = orig.mSemaphore;
			orig.mSemaphore = nullptr;
		}

		vk::Semaphore get() { return mSemaphore; }
	};
}