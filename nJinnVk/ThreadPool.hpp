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
		typedef std::function<void()> task_t;
		uint32_t mWorkerCount;
		std::thread * mWorkers;
		std::queue<task_t> mTaskQueue;
		std::mutex mMutex;
		std::condition_variable mConditionVariable;
		std::condition_variable mConditionVariableIdle;
		volatile bool mShouldRun;
		volatile uint32_t mIdleThreads;
		void workerFunction();
	public:
		ThreadPool(uint32_t workerCount);
		~ThreadPool();
		void submitTask(const task_t & task);
		void waitUntillCompleted();
	};

	extern ThreadPool * threadPool;
}
