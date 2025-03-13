#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#include "CommandBuffer.hpp"
namespace VulkanRenderer
{
	class CommandPool
	{
	public:
		CommandPool();
		CommandPool(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
		void Initialize(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
		void Shutdown();

		void AllocateBuffers(uint32_t bufferCount);

		CommandBuffer& GetCommandBuffer(uint32_t currIdx);
		
		VkCommandBuffer BeginSingleTimeCommand();
		void EndSingleTimeCommand(VkCommandBuffer commandBuffer, VkQueue graphicsQueue);
	private:
		VkDevice			mDevice;
		VkCommandPool		mCommandPool;

		std::vector<CommandBuffer> mCommandBuffers;
	};
}