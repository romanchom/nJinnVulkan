#pragma once

#include <boost/unordered_map.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <string>
#include <memory>


#include "Resource.hpp"
#include "Task.hpp"
#include "ThreadPool.hpp"

namespace nJinn {
	class ResourceManager
	{
	public:
		typedef std::string key_t;
		typedef const key_t & cref_t;
		typedef std::shared_ptr<Resource> val_t;
		typedef std::hash<key_t> hash_t;
		typedef std::equal_to<key_t> equal_t;
		typedef std::pair<const key_t, Resource> elem_t;
		typedef boost::pool_allocator<elem_t, boost::default_user_allocator_new_delete, boost::details::pool::default_mutex, 128> alloc_t;
		typedef boost::unordered_map<key_t, val_t, hash_t, equal_t, alloc_t> map_t;
	private:
		class ResourceLoadTask : public Task {
		private:
			val_t mResource;
			std::string mResourceName;
		public:
			ResourceLoadTask(val_t & resource, const std::string & resourceName) : 
				mResource(resource),
				mResourceName(resourceName) 
			{}
			virtual void execute() override { 
				mResource->load(mResourceName); 
				delete this;
			}
		};
		map_t mMap;
	public:
		template<typename T>
		std::shared_ptr<T> get(cref_t key, bool loadImmediate = false);

		void collect();
	};

	extern ResourceManager * resourceManager;

	template<typename T>
	std::shared_ptr<T> ResourceManager::get(cref_t key, bool loadImmediate)
	{
		auto it = mMap.find(key);
		if (it == mMap.end()) {
			// key not found, insert new object
			std::shared_ptr<T> value = std::make_shared<T>();
			mMap.insert(std::make_pair(key, value));
			if (loadImmediate) {
				value->load(key);
			}else{
				threadPool->submitTask(new ResourceLoadTask(std::static_pointer_cast<Resource>(value), key));
			}
			return value;
		}
		else {
			// found an object with diven key
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