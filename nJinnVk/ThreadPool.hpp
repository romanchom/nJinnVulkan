#pragma once

#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>

namespace nJinn {
	class ThreadPool
	{
	private:
		using task_t = std::function<void()>;
		std::vector<std::thread> mWorkers;
		std::queue<task_t> mTaskQueue;
		std::mutex mMutex;
		std::condition_variable mConditionVariable;
		std::condition_variable mConditionVariableIdle;
		volatile bool mShouldRun;
		volatile uint32_t mIdleThreads;
		void workerFunction();
	public:
		ThreadPool(uint32_t workerCount = 0);
		~ThreadPool();
		void submitTask(const task_t & task);
		void waitUntilCompleted();
	};

	extern ThreadPool * threadPool;
}
