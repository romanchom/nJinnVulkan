#pragma once

#include <vector>
#include <functional>
#include <mutex>

namespace nJinn {
	class Resource {
	private:
		using callback_t = std::function<void()>;
		std::mutex mCallbackMutex;
		std::vector<callback_t> mOnLoadedCallbacks;
	protected:
		bool mIsLoaded;
		void finishedLoading();
	public:
		Resource();
		virtual ~Resource() {};
		bool isLoaded() const noexcept { return mIsLoaded; }
		virtual void load(const std::string & resourceName) {};
		void onLoaded(callback_t callback);
	};
}