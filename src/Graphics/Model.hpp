#pragma once
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace VulkanRenderer
{
	struct Texture;
	struct Material
	{

	};
	struct Model
	{
		//Model data
		std::vector<glm::vec3> mVertices;
		std::vector<uint32_t> mIndices;
		Texture* mTexture;
		Material* mMaterial;

		//Vulkan handles
		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;
		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;
	};
}