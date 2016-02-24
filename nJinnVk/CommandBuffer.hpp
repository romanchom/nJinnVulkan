#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class CommandBuffer {
	public:
		CommandBuffer();
		~CommandBuffer();

		void beginRecording();

		void endRecording();
		
		operator vk::CommandBuffer() { return buffer[currentIndex]; }

		vk::CommandBuffer * get() { return &buffer[currentIndex]; }
	private:
		enum { bufferCount = 2 };
		vk::CommandPool pool;
		vk::CommandBuffer buffer[bufferCount];
		vk::CommandBufferBeginInfo beginInfo;
		size_t currentIndex;
	};

	inline void CommandBuffer::beginRecording() {
		++currentIndex %= bufferCount; // TODO possibly centralize this
		vk::resetCommandBuffer(*this, vk::CommandBufferResetFlags());
		vk::beginCommandBuffer(*this, beginInfo);
	}
	
	inline void CommandBuffer::endRecording()
	{
		vk::endCommandBuffer(*this);
	}
}