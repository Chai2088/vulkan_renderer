#include "Core/Engine.hpp"
#include "Scene/GameObject.hpp"
#include "Transform/TransformComponent.hpp"
#include "Graphics/Renderable.hpp"
#include "Resources/ResourceManager.hpp"
#include "TestScene.hpp"

namespace VulkanRenderer
{
	void TestScene::OnCreate()
	{
		Engine* engine = Engine::GetInstance();

		//Create a renderable object
		GameObject* obj = NewGameObject();
		Renderable* rd = obj->NewComp<Renderable>();
		Mesh* mesh = engine->GetResourceManager().GetResource<Mesh>("data/Models/viking_room.obj");
		Texture* tex = engine->GetResourceManager().GetResource<Texture>("data/Textures/viking_room.png");
		rd->mMesh = mesh;

		//Create texture and assign to texture
		rd->mMaterial = engine->GetFactory().Create<Material>();
		rd->mMaterial->mDiffuseTexture = tex;
		rd->mMaterial->mData.mAmbient = glm::vec3(0.1f);
		obj->OnCreate();
		obj->GetTransformComponent()->SetLocalPosition({0.0f, 0.0f, -5.0f});
		obj->GetTransformComponent()->SetLocalRotation({ 90.0f, 0.0f, 0.0f });

		//Create a light object
		GameObject* lightObj = NewGameObject();
		lightObj->NewComp<Light>();
		lightObj->OnCreate();
		TransformComponent* transform = lightObj->GetTransformComponent();
		transform->SetLocalPosition(glm::vec3(10.0f));
	}
}
