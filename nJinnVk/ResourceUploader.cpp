#include "stdafx.hpp"
#include "ResourceUploader.hpp"

#include "Context.hpp"

namespace nJinn {
	ResourceUploader * ResourceUploader::instance(nullptr);

	ResourceUploader::ResourceUploader() :
		currentIndex(0),
		tasksAdded(false)
	{
		uploadCmdBuffer.beginRecording();
	}

	ResourceUploader::~ResourceUploader()
	{
		for (size_t i = 0; i < 2; ++i) {
			for (auto & task : uploadTasks[i]) {
				vk::destroyBuffer(Context::dev(), task.buffer, nullptr);
				vk::freeMemory(Context::dev(), task.memory, nullptr);
			}
			uploadTasks[i].clear();
		}
	}

	void ResourceUploader::addTask(const uploadTask & task)
	{
		uploadTasks[currentIndex].push_back(task);
	}

	void ResourceUploader::create()
	{
		instance = new ResourceUploader();
	}

	void ResourceUploader::destroy()
	{
		delete instance;
	}

	void ResourceUploader::upload(const void * data, size_t size, vk::Buffer dst)
	{
		uploadTask task;

		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.size(size)
			.usage(vk::BufferUsageFlagBits::eTransferSrc);

		vk::createBuffer(Context::dev(), &bufferInfo, nullptr, &task.buffer);

		vk::MemoryRequirements memReq;
		vk::getBufferMemoryRequirements(Context::dev(), task.buffer, memReq);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.allocationSize(memReq.size())
			.memoryTypeIndex(Context::uploadMemoryType());

		dc(vk::allocateMemory(Context::dev(), &allocInfo, nullptr, &task.memory));

		dc(vk::bindBufferMemory(Context::dev(), task.buffer, task.memory, 0));

		void * pDest;
		vk::mapMemory(Context::dev(), task.memory, 0, size, vk::MemoryMapFlags(), &pDest);

		memcpy(pDest, data, size);

		if (!Context::isUploadMemoryTypeCoherent()) vk::flushMappedMemoryRanges(Context::dev(), 1, &vk::MappedMemoryRange(task.memory, 0, size));

		vk::cmdCopyBuffer(instance->uploadCmdBuffer, task.buffer, dst, 1, &vk::BufferCopy(0, 0, size));

		instance->addTask(task);
	}

	void ResourceUploader::doExecute()
	{
		if (tasksAdded) {
			uploadCmdBuffer.endRecording();

			vk::CommandBuffer buffer = uploadCmdBuffer;

			vk::SubmitInfo submit;
			submit
				.commandBufferCount(1)
				.pCommandBuffers(&buffer);

			vk::queueSubmit(Context::mainQueue(), 1, &submit, nullptr);
			++currentIndex %= 2;

			for (auto & task : uploadTasks[currentIndex]) {
				vk::destroyBuffer(Context::dev(), task.buffer, nullptr);
				vk::freeMemory(Context::dev(), task.memory, nullptr);
			}

			uploadTasks[currentIndex].clear();
			tasksAdded = false;

			uploadCmdBuffer.beginRecording();
		}
	}
}
