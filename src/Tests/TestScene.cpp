#include "Core/Engine.hpp"
#include "Scene/GameObject.hpp"
#include "Graphics/Renderable.hpp"
#include "Resources/ResourceManager.hpp"
#include "TestScene.hpp"

namespace VulkanRenderer
{
	void TestScene::OnCreate()
	{
		Engine* engine = Engine::GetInstance();

		GameObject* obj = NewGameObject();
		Renderable* rd = obj->NewComp<Renderable>();
		Mesh* mesh = engine->GetResourceManager().GetResource<Mesh>("data/Models/viking_room.obj");
		Texture* tex = engine->GetResourceManager().GetResource<Texture>("data/Textures/viking_room.png");
		rd->mMesh = mesh;

		//Create texture and assign to texture
		rd->mMaterial = engine->GetFactory().Create<Material>();
		rd->mMaterial->mDiffuseTexture = tex;
		obj->OnCreate();
	}
}
