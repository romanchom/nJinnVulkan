#pragma once
namespace nJinn{
	class Task
	{
	public:
		virtual void execute() = 0;
		virtual ~Task() {};
	};
}

