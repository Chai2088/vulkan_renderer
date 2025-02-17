#pragma once
#include "Core/Utils.hpp"
#include "ResourceImporter.hpp"

#include <string>
#include <map>
#include <unordered_map>
#include <typeinfo>

namespace VulkanRenderer
{
	class ResourceManager
	{
	public:
		void Initialize();
		void Shutdown();
		template <typename T>
		T* GetResource(const char* path) const
		{
			std::string typeName = ParseIDName(typeid(T).name);
			//Check if the resource list exists
			if (mResources.find(typeName) == mResources.end())
			{
				return LoadResource<T>(path);
			}
			//Resource is not registered in the list yet
			else if (mResources.at(typeName).find(path) == mResources.at(typeName).end())
			{
				return LoadResource<T>(path);
			}
			//Get the resource and return
			return reinterpret_cast<T*>(mResources.at(typeName).at(path)->GetRawResource());
		}
		template <typename T>
		T* LoadResource(const char* path)
		{
			std::string typeName = ParseIDName(typeid(T).name);
			TResource<T>* resource = mImporters.at(typeName)->ImportFromFile(path);
			mResources[typeName][path] = resource;
			return resource->mRawResource;
		}
	private:
		//mImporters[ResourceType]
		std::map<std::string, IResourceImporter*> mImporters;
		//mResources[ResourceType][ResourceName]
		std::map<std::string, std::unordered_map<std::string, IResource*>> mResources;
	};
}