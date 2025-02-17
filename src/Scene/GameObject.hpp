#pragma once
#include "Component.hpp"
#include "Core/Engine.hpp"

#include <cstdint> 
#include <unordered_map>
#include <string>
namespace VulkanRenderer
{
	class GameObject
	{
	public:
		GameObject();
		void OnCreate();
		void Initialize();
		void Shutdown();
		
		void SetID(int32_t id);
		void SetName(const char* name);

		int32_t GetId();
		std::string GetName();

		template<typename T>
		T* NewComp()
		{
			T* newComp = Engine::GetInstance()->GetFactory().Create<T>();
			AddComponent(newComp);
			return newComp;
		}
		void AddComponent(Component* comp);
	private:
		int32_t mId;
		std::string mName;

		std::unordered_map<uint32_t, Component*> mComponents;
	};
}