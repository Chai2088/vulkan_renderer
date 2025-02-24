#pragma once
#include <string>

namespace VulkanRenderer
{
	struct IResource
	{
		virtual void* GetRawResource() = 0;

		std::string mPath;
	};

	template <typename T>
	struct TResource : public IResource
	{
		void* GetRawResource() override
		{
			return mRawResource;
		}
		T* mRawResource = nullptr;
	};

	struct IResourceImporter
	{
		virtual IResource* ImportFromFile(const char* path) = 0;
		virtual void DestroyResource(IResource* res) = 0;
	};

	struct MeshImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path) override;
		void DestroyResource(IResource* res) override;
	};
	struct TextureImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path) override;
		void DestroyResource(IResource* res) override;
	};
	struct MaterialImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path) override;
		void DestroyResource(IResource* res) override;
	};
}