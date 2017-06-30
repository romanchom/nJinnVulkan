#pragma once

#include <vector>
#include <map>

namespace nJinn {
	class GameObject;
	
	class Component
	{
	private:
		GameObject * mOwner;
	public:
		virtual ~Component() {};
		void setOwner(GameObject * owner) noexcept { mOwner = owner; }
		GameObject * owner() noexcept { return mOwner; }
	};
}
