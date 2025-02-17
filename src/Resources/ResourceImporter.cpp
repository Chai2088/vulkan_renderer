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
	IResource* TextureImporter::ImportFromFile(const char* path)
	{
		TResource<Texture>* newTex = engine->GetFactory().Create<TResource<Texture>>();
		Texture* data = engine->GetRenderer().LoadTexture(path);
		newTex->mRawResource = data;
		newTex->mPath = path;
		return newTex;
	}
	IResource* MaterialImporter::ImportFromFile(const char* path)
	{
		return nullptr;
	}
}