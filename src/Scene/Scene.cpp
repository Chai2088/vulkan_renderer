#include "Core/Engine.hpp"	//Factory
#include "Scene.hpp"		

namespace VulkanRenderer
{
	GameObject* GameObjectManager::CreateGameObject(const char* name)
	{
		GameObject* newObj = Engine::GetInstance()->GetFactory().Create<GameObject>();
		
		int32_t id = GenerateId();
		std::string objName = name;
		objName += " ", std::to_string(id);
		
		newObj->SetName(objName.c_str());
		newObj->SetID(id);
		mObjects[id] = newObj;
		return newObj;
	}
	void GameObjectManager::AddGameObject(GameObject* obj)
	{
		//If there is already an object with the current id it generates a new id to the object
		if (mObjects.find(obj->GetId()) != mObjects.end() || obj->GetId() == -1)
		{
			int32_t id = GenerateId();
			obj->SetID(id);
			mObjects[obj->GetId()] = obj;
		}
		else
		{
			mObjects[obj->GetId()] = obj;
		}
	}
	void GameObjectManager::DestroyGameObject(int32_t id)
	{
		//Shutdown object and delete the object
		mObjects.at(id)->Shutdown();
		Engine::GetInstance()->GetFactory().Delete(mObjects.at(id));

		//Erase the obj from the registered objects and recycle the id
		mObjects.erase(id);
		mFreedId.push(id);
	}
	int32_t GameObjectManager::GenerateId()
	{
		//Check if a id was freed up to assign it
		if (!mFreedId.empty())
		{
			int32_t id = mFreedId.front();
			mFreedId.pop();
			return id;
		}
		//Return the next available id
		return mNextId++;
	}
	void Scene::Initialize()
	{
		for (auto& [id, obj] : mObjectManager.mObjects)
		{
			obj->Initialize();
		}
	}
	void Scene::Shutdown()
	{
		for (auto& [id, obj] : mObjectManager.mObjects)
		{
			obj->Shutdown();
		}
		mObjectManager.mObjects.clear();
	}
	GameObject* Scene::NewGameObject()
	{
		return mObjectManager.CreateGameObject("New GameObject");;
	}
	void Scene::AddGameObject(GameObject* obj)
	{
		mObjectManager.AddGameObject(obj);
	}
	void Scene::RemoveObject(uint32_t id)
	{
		mObjectManager.DestroyGameObject(id);
	}
}