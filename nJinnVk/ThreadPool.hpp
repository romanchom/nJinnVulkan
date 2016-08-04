#pragma once

#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "Task.hpp"

namespace nJinn {
	class ThreadPool
	{
	private:
		uint32_t mWorkerCount;
		std::thread * mWorkers;
		std::queue<Task *> mTaskQueue;
		std::mutex mMutex;
		std::condition_variable mConditionVariable;
		std::condition_variable mConditionVariableIdle;
		volatile bool mShouldRun;
		volatile uint32_t mIdleThreads;
		void workerFunction();
	public:
		ThreadPool(uint32_t workerCount);
		~ThreadPool();
		void submitTask(Task * t);
		void waitUntillCompleted();
	};

	extern ThreadPool * threadPool;
}
