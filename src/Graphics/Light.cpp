#include "Core/Engine.hpp"
#include "Scene/GameObject.hpp"
#include "Transform/TransformComponent.hpp"
#include "Light.hpp"

namespace VulkanRenderer
{
	LightData::LightData() : mDirection(glm::vec3(1.0f)), mColor(glm::vec3(1.0f)), mType(eLightType::ePOINT),
							 mIntensity(1.0f), mRadius(10.0f)
	{
	}
	Light::Light()
	{
		Engine::GetInstance()->GetFactory().Register<Light>();
		mName = Engine::GetInstance()->AssignComponentName<Light>();
		mId = Engine::GetInstance()->AssignComponentId<Light>();
	}
	void Light::OnCreate()
	{
	}
	void Light::Initialize()
	{
		Engine::GetInstance()->GetRenderer().AddLight(this);
		//Set the light position
		mData.mPosition = GetOwner()->GetTransformComponent()->GetWorldPosition();
	}
	void Light::Shutdown()
	{
		Engine::GetInstance()->GetRenderer().RemoveLight(this);
	}
	void Light::StreamRead(const nlohmann::json& j)
	{

	}
	void Light::StreamWrite(nlohmann::json& j) const
	{

	}
}