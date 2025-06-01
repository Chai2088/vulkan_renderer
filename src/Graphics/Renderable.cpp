#include "Core/Engine.hpp"
#include "Core/Utils.hpp"
#include "Renderable.hpp"

namespace VulkanRenderer
{
	Renderable::Renderable() : mMesh(nullptr), mMaterial(nullptr)
	{
		//Registers the component in the factory and assigns a name and id
		Engine::GetInstance()->GetFactory().Register<Renderable>();
		mId = Engine::GetInstance()->AssignComponentId<Renderable>();
		mName = "Renderable";
	}
	void Renderable::OnCreate()
	{
		//Add the renderable to the system
		Engine::GetInstance()->GetRenderer().AddToSystem(this);
	}
	void Renderable::Initialize()
	{

	}
	void Renderable::Shutdown()
	{

	}
}