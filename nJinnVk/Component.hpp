#pragma once

#include <vector>
#include <map>

namespace nJinn {
	class Component
	{
	protected:
		class GameObject * mOwner;
	public:
		virtual ~Component() {};
		void setOwner(class GameObject * owner) { mOwner = owner; }
	};
}
