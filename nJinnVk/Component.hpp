#pragma once

#include <vector>
#include <map>

namespace nJinn {
	class ComponentBase
	{
	public:
		typedef std::multimap<int, ComponentBase *> sortedComponents_t;
	protected:
		static std::vector<ComponentBase *> sUninitializedComponents;
		static void initializeNew();
		sortedComponents_t::iterator mSortedComponentsLocation;
		static sortedComponents_t sSortedComponents;
		class GameObject * mOwner;
		virtual void initialize() {};
		virtual void update() {};
	public:
		ComponentBase();
		virtual ~ComponentBase();
		void setOwner(class GameObject * owner);
		static void updateComponents();
	};

	template<int priority>
	class Component : public ComponentBase {
	public:
		Component() : 
			ComponentBase() 
		{
			mSortedComponentsLocation = sSortedComponents.insert(std::make_pair(priority, this));
		}
	};
}
