#include "GameObject.hpp"
#include "Component.hpp"
namespace VulkanRenderer
{
	GameObject* Component::GetOwner()
	{
		return mOwner;
	}
	void Component::SetOwner(GameObject* obj)
	{
		mOwner = obj;
	}
}
