#pragma once
#include "Component.hpp"
#include "Core/SystemManager.hpp"

#include <cstdint> 
#include <unordered_map>
#include <string>
namespace VulkanRenderer
{
	class GameObejct
	{
	public:
		void OnCreate();
		void Initialize();
		void Shutdown();

		template<typename T>
		T* NewComp()
		{
			T* newComp = SystemManager::GetInstance()->GetFactory().Create<T>();
			AddComponent(newComp);
			return newComp;
		}
		void AddComponent(Component* comp);
	private:
		uint32_t mId;
		std::unordered_map<uint32_t, Component*> mComponents;
	};
}