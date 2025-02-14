#pragma once
#include "Factory.hpp"	//Factory
#include "Utils.hpp"	//ParseIdName

#include <unordered_map>
namespace VulkanRenderer
{
	class SystemManager
	{
	public:
		static SystemManager* GetInstance();

		template<typename T>
		uint32_t AssignComponentId() 
		{
			std::string idName = ParseIDName(typeid(T).name());
			//Check if the component is registered
			if (mRegisteredIds.find(idName) == mRegisteredIds.end())
				mRegisteredIds[idName] = mRegisteredIds.size();

			return mRegisteredIds.at(idName);
		}

		//Get the systems
		Factory& GetFactory();
	protected:
		//Singleton
		SystemManager() {}
		SystemManager(const SystemManager&) = delete;
		void operator=(const SystemManager&) = delete;
	
		static SystemManager* mSystemManager;
	private:
		Factory mFactory;
		
		//Ids
		std::unordered_map<std::string, uint32_t> mRegisteredIds;
	};
}