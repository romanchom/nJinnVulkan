#pragma once

#include <vulkan.hpp>

#include "Context.hpp"

namespace nJinn {
	class Fence {
	private:
		vk::Fence mFence;
	public:
		Fence(bool signaled = false) {
			VkFenceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = signaled;
			mFence = context->dev().createFence(info);
		};
		~Fence() {
			context->dev().destroyFence(mFence);
		};
		operator vk::Fence &() {
			return mFence;
		}
		vk::Fence operator->() { return mFence; }
	};
}