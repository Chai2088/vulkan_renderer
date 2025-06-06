#pragma once


#include <vulkan/vulkan.hpp>
#include <string>

#include "GraphicsPipeline.hpp"
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
		int32_t mMatIdx;
	};
	struct Model
	{
		Model();
		//Vulkan buffer for instances
		VkBuffer mInstanceBuffer;
		VkDeviceMemory mInstanceBufferMemory;
		uint32_t mInstanceCount;

		std::vector<Mesh*> mMeshes;
		std::vector<Material*> mMats;

		//Updates the instance buffer
		void UpdateInstanceBuffer(VkDevice device, const std::vector<InstanceData>& instances);

		//Load the model -> create all the meshes and load all the materials
		void LoadModel(const char* filename);
		//TODO: Take necessary input to submit render command
		void Draw(CommandBuffer& commandBuffer, GraphicsPipeline& pipeline, VkBuffer instanceBuffer, uint32_t instances);
	};
}