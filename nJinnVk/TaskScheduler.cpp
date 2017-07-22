#include "stdafx.hpp"
#include "TaskScheduler.hpp"

#include "ThreadPool.hpp"

namespace nJinn {
	TaskScheduler::TaskScheduler(ThreadPool * pool) :
		mThreadPool(pool)
	{}

	TaskScheduler::TaskHandle TaskScheduler::schedule(const std::function<void()>& function, std::vector<TaskHandle> dependencies)
	{
		mTasks.emplace_front();
		TaskHandle task = mTasks.begin();

		task->function = function;
		task->unmetDependencyCount.store(static_cast<int>(dependencies.size()));
		for (TaskHandle dep : dependencies) {
			dep->dependants.push_back(task);
		}

		return task;
	}

	void TaskScheduler::reset() {
		for (auto && task : mTasks) {
			task.unmetDependencyCount = task.dependencyCount;
		}
	}

	void TaskScheduler::execute() {
		for (auto && task : mTasks) {
			if (0 == task.unmetDependencyCount.load()) {
				startTask(task);
			}
		}
	}

	void TaskScheduler::startTask(Task & task) {
		mThreadPool->submitTask([&task, this]() {
			task.function();
			this->startDependencies(task);
		});
	}

	void TaskScheduler::startDependencies(Task & task) {
		for (auto && dependant : task.dependants) {
			int dependencyCount = --dependant->unmetDependencyCount;
			if (0 == dependencyCount) {
				this->startTask(*dependant);
			}
		}
	}

}