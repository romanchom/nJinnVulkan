#pragma once

#include <vulkan.hpp>

#include "Context.hpp"

namespace nJinn {
	class Fence {
	private:
		vk::Fence mFence;
		Fence(const Fence &) = delete;
		Fence & operator=(const Fence &) = delete;
		void destroy() {
			if(nullptr != mFence)
				context->dev().destroyFence(mFence);
		}
	public:
		Fence() : mFence(nullptr) {}
		explicit Fence(bool signaled) {
			vk::FenceCreateInfo info;
			if(signaled)
				info.setFlags(vk::FenceCreateFlagBits::eSignaled);
			mFence = context->dev().createFence(info);
		};
		Fence(Fence && orig) :
			mFence(orig.mFence)
		{
			orig.mFence = nullptr;
		}

		Fence & operator=(Fence && orig) {
			destroy();
			mFence = orig.mFence;
			orig.mFence = nullptr;
		}

		~Fence() {
			destroy();
		};

		vk::Fence get() { return mFence; }
	};
}