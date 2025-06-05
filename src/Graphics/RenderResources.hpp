#pragma once


#include <vulkan/vulkan.hpp>
#include <string>

#include "CommandBuffer.hpp"
#include "Material.hpp"

namespace VulkanRenderer
{
	struct Texture
	{
		std::string mTexturePath;
		uint32_t mWidth, mHeight;

		uint32_t mMipLevels;
		VkImageView mTextureImageView;
		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
	};
	struct Mesh
	{
		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;
		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;

		uint32_t mIndexCount;
		//Every mesh has its own material
		uint32_t mMatIdx;
	};
	struct Model
	{
		std::vector<Mesh*> mMeshes;
		std::vector<Material*> mMats;

		//Load the model -> create all the meshes and load all the materials
		void LoadModel(const char* filename);
		//TODO: Take necessary input to submit render command
		void Draw(CommandBuffer& commandBuffer);
	};
}