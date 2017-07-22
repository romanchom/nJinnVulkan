#pragma once

#include <functional>
#include <initializer_list>
#include <atomic>
#include <forward_list>

namespace nJinn {
	class TaskScheduler
	{
	private:
		struct Task;
		std::forward_list<Task> mTasks;
		using TaskHandle = std::forward_list<Task>::iterator;

		struct Task {
			std::function<void()> function;
			std::vector<TaskHandle> dependants;
			int dependencyCount;
			std::atomic<int> unmetDependencyCount;
			Task() :
				unmetDependencyCount(0)
			{}
			Task(const Task & t) :
				function(t.function),
				dependants(t.dependants),
				dependencyCount(t.dependencyCount),
				unmetDependencyCount(t.unmetDependencyCount.load())
			{}
		};
		class ThreadPool * mThreadPool;
	public:
		TaskScheduler(class ThreadPool * pool);
		TaskHandle schedule(const std::function<void()> & function, std::vector<TaskHandle> dependencies);
		void reset();
		void execute();
	private:
		void startTask(Task & task);
		void startDependencies(Task & task);
	};
}
