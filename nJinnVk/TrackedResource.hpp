#pragma once

#include <memory>
#include <map>

namespace nJinn {
	template<typename T, typename key_t = std::string>
	class TrackedResource {
	public:
		typedef std::shared_ptr<T> p;
		static std::shared_ptr<T> load(const key_t & name);
		static unsigned int collect();
	protected:
		TrackedResource() {};
	private:
		static std::map<key_t, std::shared_ptr<T>> sResources;
	};

	template<typename T, typename key_t>
	std::map<key_t, std::shared_ptr<T>> TrackedResource<T, key_t>::sResources;

	template<typename T, typename key_t>
	std::shared_ptr<T> TrackedResource<T, key_t>::load(const key_t & name)
	{
		auto it = sResources.find(name);
		if (it != sResources.end()) {
			return it->second;
		} else {
			std::shared_ptr<T> me = std::make_shared<T>(name);
			sResources.insert(std::make_pair(name, me));
			return me;
		}
	}

	template<typename T, typename key_t>
	unsigned int TrackedResource<T, key_t>::collect()
	{
		unsigned int count = 0;
		auto it = sResources.begin();
		while (it != sResources.end()) {
			auto current = it;
			++it;
			if (current->second.unique()) {
				++count;
				sResources.erase(current);
			}
		}
		return count;
	}
}