#include "stdafx.hpp"
#include "ThreadPool.hpp"

using namespace std;

namespace nJinn {
	ThreadPool * threadPool;
	void ThreadPool::workerFunction()
	{
		while (true) {
			Task * t;
			{
				unique_lock<mutex> lock(mMutex);
				while (mTaskQueue.empty() && mShouldRun) {
					++mIdleThreads;
					if((mTaskQueue.empty()) && (mIdleThreads == mWorkerCount)) mConditionVariableIdle.notify_one();
					mConditionVariable.wait(lock);
					--mIdleThreads;
				}
				if (!mShouldRun) break;

				t = mTaskQueue.front();
				mTaskQueue.pop();
			}
			t->execute();
		}
	}

	ThreadPool::ThreadPool(uint32_t workerCount) : 
		mWorkerCount(workerCount),
		mWorkers(new std::thread[workerCount]),
		mShouldRun(true),
		mIdleThreads(0)
	{
		for (int i = 0; i < workerCount; ++i) {
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

	void ThreadPool::submitTask(Task * t)
	{
		unique_lock<mutex> lock(mMutex);

		mTaskQueue.push(t);
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