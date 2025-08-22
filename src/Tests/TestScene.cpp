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

		//Create an object
		GameObject* obj = NewGameObject();
		obj->OnCreate();
		TransformComponent* transform = obj->GetTransformComponent();
		transform->SetLocalScale(glm::vec3(1000.0f, 1.0f, 1000.0f));
		Renderable* rd = obj->NewComp<Renderable>();
		Model* mesh = engine->GetResourceManager().GetResource<Model>("data/Models/cube.obj");
		rd->mModel = mesh;
		rd->mColor = glm::vec3(0.0f, 1.0f, 1.0f);


		//Create lights
		GameObject* light0 = NewGameObject();
		light0->OnCreate();

		TransformComponent* lightTransform = light0->GetTransformComponent(); 
		lightTransform->SetLocalPosition(glm::vec3(300.0f, 300.0f, 3.0f));
		lightTransform->SetLocalScale(glm::vec3(10.0f));
		Light* lightComp = light0->NewComp<Light>();
		lightComp->mData.mColor = glm::vec3(1.0f, 1.0f, 1.0f);
		lightComp->mData.mDirection = glm::vec3(-1.0f, -1.0f, 1.0f);
		lightComp->mData.mIntensity = 1.0f;
		lightComp->mData.mType = 1;

		mesh = engine->GetResourceManager().GetResource<Model>("data/Models/wooden_sphere.obj");
		rd = light0->NewComp<Renderable>();
		rd->mModel = mesh;
		rd->mColor = glm::vec3(1.0f, 1.0f, 1.0f);

		//Create the sphere where it casts the shadow
		GameObject* sphere = NewGameObject();
		sphere->OnCreate();
		lightTransform = sphere->GetTransformComponent();
		lightTransform->SetLocalPosition(glm::vec3(0.0f, 100.0f,0.0f));
		lightTransform->SetLocalScale(glm::vec3(10.0f));
		rd = sphere->NewComp<Renderable>();
		rd->mModel = mesh;
		rd->mColor = glm::vec3(1.0f, 0.0f, 1.0f);
	}
}
