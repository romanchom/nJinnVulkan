#include "stdafx.hpp"
#include "Resource.hpp"

#include "ResourceManager.hpp"

namespace nJinn {
	Resource::Resource() :
		mIsLoaded(false)
	{}

	void Resource::onLoaded(callback_t callback) {
		std::lock_guard<std::mutex> lock(mCallbackMutex);
		if (mIsLoaded) {
			callback();
		} else {
			mOnLoadedCallbacks.emplace_back(callback);
		}
	}

	void Resource::finishedLoading() {
		std::lock_guard<std::mutex> lock(mCallbackMutex);
		mIsLoaded = true;
		for (auto && callback : mOnLoadedCallbacks) {
			callback();
		}
		// move empty vector into member to release memory
		mOnLoadedCallbacks = std::vector<std::function<void()>>();
	}
}


