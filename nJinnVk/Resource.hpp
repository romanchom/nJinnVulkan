#pragma once


namespace nJinn {
	class Resource {
	protected:
		bool mIsLoaded;
	public:
		Resource();
		virtual ~Resource() {};
		bool isLoaded() const { return mIsLoaded; }
		virtual void load(const std::string & resourceName) {};
	protected:
		void finishedLoading();
	};
}