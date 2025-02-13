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
}

