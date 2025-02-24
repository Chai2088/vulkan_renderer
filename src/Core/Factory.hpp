#pragma once
#include "Utils.hpp"

#include <map>
#include <string>
#include <typeinfo>

namespace VulkanRenderer
{
	//Interface class
	struct ICreators
	{
		virtual void* Create() = 0;
	};
	//Template class 
	template<typename T>
	struct TCreators : ICreators
	{
		void* Create()
		{
			return new T();
		}
	};
	class Factory
	{
	public:
		void Shutdown();
		void Delete(void* obj);

		//Resgister and creates intances of objects
		template<typename T>
		void Register()
		{
			std::string idName = ParseIDName(typeid(T).name());

			//Check if the component was already registered
			if (mCreators.find(idName) != mCreators.end())
				return;

			//Register the new creator
			mCreators[idName] = new TCreators<T>();
		}

		template<typename T>
		T* Create()
		{
			std::string idName = ParseIDName(typeid(T).name());

			//Check if the component was already registered, if not it registers it
			if (mCreators.find(idName) == mCreators.end())
				Register<T>();

			return reinterpret_cast<T*>(mCreators.at(idName)->Create());
		}
	private:
		std::map<std::string, ICreators*> mCreators;
	};
}