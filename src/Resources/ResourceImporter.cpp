#include "Core/Engine.hpp"
#include "Graphics/RenderResources.hpp"
#include "Graphics/Renderer.hpp"
#include "ResourceImporter.hpp"

namespace
{
	VulkanRenderer::Engine* engine = VulkanRenderer::Engine::GetInstance();
}

namespace VulkanRenderer
{
	IResource* MeshImporter::ImportFromFile(const char* path)
	{
		TResource<Mesh>* newMesh = engine->GetFactory().Create<TResource<Mesh>>();
		Mesh* data = engine->GetRenderer().LoadMesh(path);
		newMesh->mRawResource = data;
		newMesh->mPath = path;
		return newMesh;
	}
	void MeshImporter::DestroyResource(IResource* res)
	{
		Mesh* data = reinterpret_cast<Mesh*>(res->GetRawResource());
		engine->GetRenderer().DestroyMesh(data);
	}
	IResource* TextureImporter::ImportFromFile(const char* path)
	{
		TResource<Texture>* newTex = engine->GetFactory().Create<TResource<Texture>>();
		Texture* data = engine->GetRenderer().LoadTexture(path);
		newTex->mRawResource = data;
		newTex->mPath = path;
		return newTex;
	}
	void TextureImporter::DestroyResource(IResource* res)
	{
		Texture* data = reinterpret_cast<Texture*>(res->GetRawResource());
		engine->GetRenderer().DestroyTexture(data);
	}
	IResource* MaterialImporter::ImportFromFile(const char* path)
	{
		return nullptr;
	}
	void MaterialImporter::DestroyResource(IResource* res)
	{
	}
}