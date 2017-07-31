#include "stdafx.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	ResourceManager * resourceManager;

	void ResourceManager::collect()
	{
		auto it = mMap.begin();
		while (mMap.end() != it) {
			auto current = it++;
			if (current->second.unique()) mMap.erase(current);
		}
	}
}

