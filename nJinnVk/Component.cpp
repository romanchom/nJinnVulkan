#include "stdafx.hpp"
#include "Component.hpp"

namespace nJinn {
	std::vector<ComponentBase *> ComponentBase::sUninitializedComponents;
	ComponentBase::sortedComponents_t ComponentBase::sSortedComponents;

	ComponentBase::ComponentBase()
	{
		sUninitializedComponents.push_back(this);
	}

	ComponentBase::~ComponentBase()
	{
		sSortedComponents.erase(mSortedComponentsLocation);
	}

	void ComponentBase::initializeNew()
	{
		for (auto comp : sUninitializedComponents) {
			comp->initialize();
		}
		sUninitializedComponents.clear();
	}

	void ComponentBase::updateComponents()
	{
		initializeNew();
		for (auto item : sSortedComponents) {
			item.second->update();
		}
	}

	void ComponentBase::setOwner(class GameObject * owner) {
		mOwner = owner;
	}
}
