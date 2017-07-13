#pragma once

#include <unordered_map>
#include <boost/pool/pool_alloc.hpp>
#include <string>
#include <memory>
#include <functional>

#include "Resource.hpp"
#include "ThreadPool.hpp"
#include "Debug.hpp"
#include "Clock.hpp"

namespace nJinn {
	enum class ResourceLoadPolicy {
		None,
		Deferred,
		Immediate,
	};
	class ResourceManager
	{
	public:
		using key_t = std::string;
		template<typename T>
		using handle_t = std::shared_ptr<T>;
		using val_t = std::shared_ptr<Resource>;
		using hash_t = std::hash<key_t>;
		using equal_t = std::equal_to<key_t>;
		using elem_t = std::pair<const key_t, Resource>;
		using alloc_t = boost::pool_allocator<elem_t, boost::default_user_allocator_new_delete, boost::details::pool::default_mutex, 128>;
		using map_t = std::unordered_map<key_t, val_t, hash_t, equal_t, alloc_t>;
		using callback_t = std::function<bool()>;
		using callbackMap_t = std::unordered_multimap <Resource *, callback_t, std::hash<Resource *>, std::equal_to<Resource *>, boost::pool_allocator<std::pair<Resource *, callback_t>>>;
	private:
		map_t mMap;
		callbackMap_t mOnResourceLoadedCallbacks;
	public:
		template<typename T>
		handle_t<T> get(const key_t & key, ResourceLoadPolicy policy = ResourceLoadPolicy::None);

		void onResourceLoaded(const val_t & resource, const callback_t & callback);
		void runCallbacks(Resource * resource);

		void collect();
	};

	extern ResourceManager * resourceManager;

	template<typename T>
	ResourceManager::handle_t<T> ResourceManager::get(const key_t & key, ResourceLoadPolicy policy)
	{
		auto it = mMap.find(key);
		if (it == mMap.end()) {
			// key not found, insert new object
			std::shared_ptr<T> resource = std::make_shared<T>();
			mMap.emplace(key, resource);
			switch (policy) {
			case ResourceLoadPolicy::Immediate:
				resource->load(key);
				break;
			case ResourceLoadPolicy::Deferred:
				threadPool->submitTask([=]() {
					resource->load(key); 
					debug->log("Resource loaded in frame ", clock->frame(), '\n');
				});
				break;
			}
			return resource;
		}
		else {
			// found an object with given key
			val_t & foundResource = it->second;
			std::shared_ptr<T> value = std::dynamic_pointer_cast<T>(foundResource);
			if (nullptr == value) {
				// it is of different type
				throw std::runtime_error("Resource is of wrong type.");
			}else{
				// it is of correct type
				return value;
			}
		}
	}
}