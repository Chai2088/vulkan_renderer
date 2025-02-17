#include "GameObject.hpp"

namespace VulkanRenderer
{
	GameObject::GameObject(): mId(-1), mName("New GameObject")
	{
		
	}
	void GameObject::OnCreate()
	{
		for (auto& comp : mComponents)
		{
			comp.second->OnCreate();
		}
	}

	void GameObject::Initialize()
	{
		for (auto& comp : mComponents)
		{
			comp.second->Initialize();
		}
	}

	void GameObject::Shutdown()
	{
		for (auto& comp : mComponents)
		{
			comp.second->Shutdown();
		}
	}
	void GameObject::SetID(int32_t id)
	{
		mId = id;
	}
	void GameObject::SetName(const char* name)
	{
		mName = name;
	}
	int32_t GameObject::GetId()
	{
		return mId;
	}
	std::string GameObject::GetName()
	{
		return mName;
	}
	void GameObject::AddComponent(Component* comp)
	{
		//Check if the component already exist in the object
		if (mComponents.find(comp->GetId()) != mComponents.end())
			return;

		//Register the component
		mComponents[comp->GetId()] = comp;
	}
}

