#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
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
}