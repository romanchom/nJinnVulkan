#include "stdafx.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	ResourceManager * resourceManager;

	void ResourceManager::onResourceLoaded(const val_t & resource, callback_t callback)
	{
		if (resource->isLoaded()) {
			callback();
		}
		else {
			mOnResourceLoadedCallbacks.emplace(std::make_pair(resource.get(), callback));
		}
	}

	void ResourceManager::runCallbacks(Resource * resource)
	{
		auto its = mOnResourceLoadedCallbacks.equal_range(resource);
		for (auto it = its.first; it != its.second; ++it) {
			it->second();
		}
		mOnResourceLoadedCallbacks.erase(its.first, its.second);
	}

	void ResourceManager::collect()
	{
		auto it = mMap.begin();
		while (mMap.end() != it) {
			auto current = it++;
			if (current->second.unique()) mMap.erase(current);
		}
	}
}

