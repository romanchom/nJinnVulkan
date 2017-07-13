#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class CommandBuffer {
	public:
		CommandBuffer();
		~CommandBuffer();

		inline void beginRecording();

		inline void endRecording();
		
		operator vk::CommandBuffer() { return buffer[currentIndex]; }

		vk::CommandBuffer get() { return buffer[currentIndex]; }
		vk::CommandBuffer * operator->() {
			return buffer + currentIndex;
		}
	private:
		enum { bufferCount = 3 };
		vk::CommandPool pool;
		vk::CommandBuffer buffer[bufferCount];
		vk::CommandBufferBeginInfo beginInfo;
		size_t currentIndex;
	};

	inline void CommandBuffer::beginRecording() {
		++currentIndex %= bufferCount; // TODO possibly centralize this
		get().reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		get().begin(beginInfo);
	}
	
	inline void CommandBuffer::endRecording()
	{
		get().end();
	}
}