#pragma once
#include <string>

namespace VulkanRenderer
{
	struct IResource
	{
		virtual void* GetRawResource() {}

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
	};

	struct MeshImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path);
	};
	struct TextureImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path);
	};
	struct MaterialImporter : public IResourceImporter
	{
		IResource* ImportFromFile(const char* path);
	};
}