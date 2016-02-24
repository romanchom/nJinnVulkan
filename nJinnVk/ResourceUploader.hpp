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
		ResourceUploader();
		~ResourceUploader();

		CommandBuffer uploadCmdBuffer;

		static ResourceUploader * instance;
		std::vector<uploadTask> uploadTasks[2];
		size_t currentIndex;
		Semaphore transfersCompleteSemaphore;
		bool tasksAdded;
		void addTask(const uploadTask & task);
		void doExecute();
	public:
		static void create();
		static void destroy();

		static void upload(const void * data, size_t size, vk::Buffer dst);
		static void execute() { instance->doExecute(); }
		static vk::Semaphore semaphore() { return instance->transfersCompleteSemaphore; }
	};
}