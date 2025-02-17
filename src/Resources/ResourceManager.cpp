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
	}
}