#pragma once
#include "Core/Engine.hpp"
#include "Component.hpp"

#include <cstdint> 
#include <unordered_map>
#include <string>
namespace VulkanRenderer
{
	class TransformComponent;
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

		TransformComponent* GetTransformComponent() const;

		template<typename T>
		T* GetComponent()
		{
			uint32_t compId = Engine::GetInstance()->AssignComponentId<T>();
			if (mComponents.find(compId) == mComponents.end())
				return nulltpr;
			return mComponents.at(compId);
		}

		template<typename T>
		T* NewComp()
		{
			T* newComp = Engine::GetInstance()->GetFactory().Create<T>();
			try
			{
				Component* comp = dynamic_cast<Component*>(newComp);
				AddComponent(newComp);
				newComp->OnCreate();
				return newComp;
			}
			catch (const std::bad_cast& e)
			{
				Engine::GetInstance()->GetFactory().Delete(newComp);
				return nullptr;
			}
		}
		void AddComponent(Component* comp);
	
	private:
		int32_t mId;
		std::string mName;

		TransformComponent* mTransformComp;
		std::unordered_map<uint32_t, Component*> mComponents;
	};
}