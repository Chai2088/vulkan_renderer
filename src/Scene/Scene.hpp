#pragma once
#include "GameObject.hpp"

#include <unordered_map>
#include <queue>

namespace VulkanRenderer
{
	struct GameObjectManager
	{
		//Methods
		GameObject* CreateGameObject(const char* name);
		void AddGameObject(GameObject* obj);
		void DestroyGameObject(int32_t id);
		int32_t GenerateId();

		//data
		std::unordered_map<int32_t, GameObject*> mObjects;
		std::queue<int32_t> mFreedId;
		int32_t mNextId = 0;
	};

	class Scene
	{
	public:
		void Initialize();
		void Shutdown();

		GameObject* NewGameObject();
		void AddGameObject(GameObject* obj);
		void RemoveObject(uint32_t id);
	private:
		GameObjectManager mObjectManager;
	};
}