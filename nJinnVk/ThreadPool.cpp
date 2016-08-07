#include "stdafx.hpp"
#include "ThreadPool.hpp"

using namespace std;

namespace nJinn {
	ThreadPool * threadPool;
	void ThreadPool::workerFunction()
	{
		while (true) {
			task_t task;
			{
				unique_lock<mutex> lock(mMutex);
				while (mTaskQueue.empty() && mShouldRun) {
					++mIdleThreads;
					if((mTaskQueue.empty()) && (mIdleThreads == mWorkerCount)) mConditionVariableIdle.notify_one();
					mConditionVariable.wait(lock);
					--mIdleThreads;
				}
				if (!mShouldRun) break;

				task = mTaskQueue.front();
				mTaskQueue.pop();
			}
			task();
		}
	}

	ThreadPool::ThreadPool(uint32_t workerCount) : 
		mWorkerCount(workerCount),
		mWorkers(new std::thread[workerCount]),
		mShouldRun(true),
		mIdleThreads(0)
	{
		for (uint32_t i = 0; i < workerCount; ++i) {
			mWorkers[i] = std::thread(&ThreadPool::workerFunction, this);
		}
	}

	ThreadPool::~ThreadPool()
	{
		mShouldRun = false;
		mConditionVariable.notify_all();
		for (unsigned i = 0; i < mWorkerCount; ++i) {
			mWorkers[i].join();
		}
		delete[] mWorkers;
	}

	void ThreadPool::submitTask(const task_t & task)
	{
		unique_lock<mutex> lock(mMutex);
		mTaskQueue.push(task);
		mConditionVariable.notify_one();
	}

	void ThreadPool::waitUntillCompleted()
	{
		unique_lock<mutex> lock(mMutex);
		while ((!mTaskQueue.empty()) || (mIdleThreads != mWorkerCount)) {
			mConditionVariableIdle.wait(lock);
		}
	}
}