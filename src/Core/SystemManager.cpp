#include "SystemManager.hpp"
namespace VulkanRenderer
{
	SystemManager* SystemManager::mSystemManager = nullptr;

	SystemManager* SystemManager::GetInstance()
	{
		if (mSystemManager == nullptr)
			mSystemManager = new SystemManager();
		return mSystemManager;
	}
	Factory& SystemManager::GetFactory()
	{
		return mFactory;
	}
}