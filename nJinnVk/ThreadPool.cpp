#include "stdafx.hpp"
#include "ThreadPool.hpp"

namespace nJinn {
	ThreadPool * threadPool;
	void ThreadPool::workerFunction()
	{
		while (true) {
			task_t task;
			{
				std::unique_lock<std::mutex> lock(mMutex);
				while (mTaskQueue.empty() && mShouldRun.load()) {
					mIdleThreads++;
					if((mTaskQueue.empty()) && (mIdleThreads.load() == mWorkers.size())) mConditionVariableIdle.notify_one();
					mConditionVariable.wait(lock);
					mIdleThreads--;
				}
				if (!mShouldRun.load()) break;

				task = mTaskQueue.front();
				mTaskQueue.pop();
			}
			task();
		}
	}

	ThreadPool::ThreadPool(uint32_t workerCount) : 
		mWorkers(),
		mShouldRun(true),
		mIdleThreads(0)
	{
		if (0 == workerCount) {
			workerCount = std::thread::hardware_concurrency();
			// if optimal number of threads cannot be determined
			// assume 4 threads
			if (0 == workerCount) workerCount = 4;
		}

		for (uint32_t i = 0; i < workerCount; ++i) {
			mWorkers.emplace_back(&ThreadPool::workerFunction, this);
		}
	}

	ThreadPool::~ThreadPool()
	{
		mShouldRun.store(false);
		mConditionVariable.notify_all();
		for (auto && worker : mWorkers) {
			worker.join();
		}
	}

	void ThreadPool::submitTask(const task_t & task)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mTaskQueue.push(task);
		mConditionVariable.notify_one();
	}

	void ThreadPool::waitUntilCompleted()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while ((!mTaskQueue.empty()) || (mIdleThreads != mWorkers.size())) {
			mConditionVariableIdle.wait(lock);
		}
	}
}