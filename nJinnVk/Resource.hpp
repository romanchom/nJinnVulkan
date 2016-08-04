#pragma once


namespace nJinn {
	class Resource {
	protected:

	public:
		virtual ~Resource() {};
		virtual void load(const std::string & resourceName) {};
	};
}