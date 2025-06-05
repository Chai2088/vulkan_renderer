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
	IResource* ModelImporter::ImportFromFile(const char* path)
	{
		TResource<Model>* newModel = engine->GetFactory().Create<TResource<Model>>();
		//Create a model object and load the model
		Model* model = engine->GetFactory().Create<Model>();
		model->LoadModel(path);
		//Assign the model to the resource object
		newModel->mRawResource = model;
		return newModel;
	}
	void ModelImporter::DestroyResource(IResource* res)
	{
		Model* data = reinterpret_cast<Model*>(res->GetRawResource());
		for (uint32_t i = 0; i < data->mMeshes.size(); ++i)
		{
			Engine::GetInstance()->GetRenderer().DestroyMesh(data->mMeshes[i]);
		}

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