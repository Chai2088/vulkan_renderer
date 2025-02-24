#include "Core/Engine.hpp"
#include "ResourceManager.hpp"

namespace VulkanRenderer
{
	void ResourceManager::Initialize()
	{
		Engine* engine = Engine::GetInstance();
		//Register all the importes
		mImporters["Mesh"] = engine->GetFactory().Create<MeshImporter>();
		mImporters["Texture"] = engine->GetFactory().Create<TextureImporter>();
	}
	void ResourceManager::Shutdown()
	{
		Engine* engine = Engine::GetInstance();
		//Renderer waits in idle as it is detroying the resource buffers
		engine->GetRenderer().WaitIdle();

		//Clear all the resources
		for (auto& [type, list] : mResources)
		{
			for (auto& [resName, res] : list)
			{
				mImporters.at(type)->DestroyResource(res);
				engine->GetFactory().Delete(res);
			}
		}
		//Clear all the importers
		for (auto& [type, importer] : mImporters)
		{
			engine->GetFactory().Delete(importer);
		}
	}
}