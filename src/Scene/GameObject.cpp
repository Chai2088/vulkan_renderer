#include "GameObject.hpp"

namespace VulkanRenderer
{
	void GameObejct::OnCreate()
	{
		for (auto& comp : mComponents)
		{
			comp.second->OnCreate();
		}
	}

	void GameObejct::Initialize()
	{
		for (auto& comp : mComponents)
		{
			comp.second->Initialize();
		}
	}

	void GameObejct::Shutdown()
	{
		for (auto& comp : mComponents)
		{
			comp.second->Shutdown();
		}
	}
	void GameObejct::AddComponent(Component* comp)
	{
		//Check if the component already exist in the object
		if (mComponents.find(comp->GetId()) != mComponents.end())
			return;

		//Register the component
		mComponents[comp->GetId()] = comp;
	}
}

