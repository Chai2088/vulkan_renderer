#include "Core/SystemManager.hpp"
#include "Core/Utils.hpp"
#include "Renderable.hpp"

namespace VulkanRenderer
{
	Renderable::Renderable()
	{
		//Registers the component in the factory and assigns a name and id
		SystemManager::GetInstance()->GetFactory().Register<Renderable>();
		mId = SystemManager::GetInstance()->AssignComponentId<Renderable>();
		mName = "Renderable";
	}
	void Renderable::OnCreate()
	{

	}
	void Renderable::Initialize()
	{

	}
	void Renderable::Shutdown()
	{

	}
}