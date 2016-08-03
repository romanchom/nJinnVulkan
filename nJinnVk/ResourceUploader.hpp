#pragma once

#include <vector>

#include <vulkan.hpp>

#include "CommandBuffer.hpp"
#include "Semaphore.hpp"

namespace nJinn {
	class ResourceUploader {
	private:
		struct uploadTask {
			vk::Buffer buffer;
			vk::DeviceMemory memory;
		};

		CommandBuffer uploadCmdBuffer;

		std::vector<uploadTask> uploadTasks[2];
		size_t currentIndex;
		Semaphore transfersCompleteSemaphore;
		bool tasksAdded;
		void addTask(const uploadTask & task);
		void doExecute();
	public:
		ResourceUploader();
		~ResourceUploader();

		void upload(const void * data, size_t size, vk::Buffer dst);
		void execute() { doExecute(); }
		vk::Semaphore semaphore() { return transfersCompleteSemaphore; }
	};

	extern ResourceUploader * resourceUploader;
}