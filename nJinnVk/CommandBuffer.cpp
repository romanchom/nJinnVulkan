#include "stdafx.hpp"
#include "CommandBuffer.hpp"

#include "Context.hpp"

namespace nJinn {
	nJinn::CommandBuffer::CommandBuffer() :
		currentIndex(-1)
	{

		uint32_t flag = static_cast<uint32_t>(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo
			.flags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.queueFamilyIndex(Context::mainQueueFamilyIndex());
			
		dc(vk::createCommandPool(Context::dev(), &poolInfo, nullptr, &pool));

		vk::CommandBufferAllocateInfo bufferInfo;
		bufferInfo
			.commandBufferCount(bufferCount)
			.commandPool(pool)
			.level(vk::CommandBufferLevel::ePrimary);

		dc(vk::allocateCommandBuffers(Context::dev(), &bufferInfo, buffer));

		beginInfo.flags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	}

	CommandBuffer::~CommandBuffer()
	{
		vk::destroyCommandPool(Context::dev(), pool, nullptr);
	}
}