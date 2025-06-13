#pragma once
#include <vulkan/vulkan.hpp>
namespace VulkanRenderer
{
	struct Buffer
	{
		VkBuffer buffer{};
		VkDeviceMemory memory{};
		void* bufferMapped = nullptr;
	};
}