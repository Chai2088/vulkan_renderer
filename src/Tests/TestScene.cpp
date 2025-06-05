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

		////Create a renderable object
		//GameObject* obj = NewGameObject();
		//Renderable* rd = obj->NewComp<Renderable>();
		////Mesh* mesh = engine->GetResourceManager().GetResource<Mesh>("data/Models/viking_room.obj");
		//Texture* tex = engine->GetResourceManager().GetResource<Texture>("data/Textures/viking_room.png");
		////rd->mMesh = mesh;
		//Model* model = engine->GetResourceManager().GetResource<Model>("data/Models/viking_room.obj");
		//model->mMeshes[0]->mMat->mDiffuseTexName = "data/Textures/viking_room.png";
		//model->mMeshes[0]->mMat->mData.mAmbient = glm::vec3(0.1f);
		//obj->OnCreate();
		//obj->GetTransformComponent()->SetLocalPosition({0.0f, 0.0f, -5.0f});
		//obj->GetTransformComponent()->SetLocalRotation({ 90.0f, 0.0f, 0.0f });

		//Create a light object
		GameObject* lightObj = NewGameObject();
		Light* light = lightObj->NewComp<Light>();
		light->mData.mIntensity = 100;
		lightObj->OnCreate();
		TransformComponent* transform = lightObj->GetTransformComponent();
		transform->SetLocalPosition(glm::vec3(50.0f));
		transform->SetLocalScale(glm::vec3(0.1f));
		//Add a renderable and assign a sphere model
		Renderable* rd = lightObj->NewComp<Renderable>();
		Model* mesh = engine->GetResourceManager().GetResource<Model>("data/Models/sponza.obj");
		rd->mModel = mesh;
	}
}
