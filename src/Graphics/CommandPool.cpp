#include "CommandPool.hpp"

namespace VulkanRenderer
{
	CommandPool::CommandPool()
	{
	}

	CommandPool::CommandPool(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
	{
		Initialize(device, flags, queueFamilyIndex);
	}

	void CommandPool::Initialize(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
	{
		mDevice = device;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.queueFamilyIndex = queueFamilyIndex;

		if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void CommandPool::Shutdown()
	{
		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	}

	void CommandPool::AllocateBuffers(uint32_t bufferCount)
	{
		std::vector<VkCommandBuffer> commandBuffers(bufferCount);
		//Allocates the amount of command buffers specified
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = bufferCount;

		if (vkAllocateCommandBuffers(mDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		//Set the command buffer handle to the Command Buffer class
		mCommandBuffers.resize(bufferCount);
		for(uint32_t i = 0; i < bufferCount; ++i)
		{
			mCommandBuffers.at(i).Initialize(commandBuffers.at(i));
		}
	}

	CommandBuffer& CommandPool::GetCommandBuffer(uint32_t currIdx)
	{
		return mCommandBuffers.at(currIdx);
	}

	VkCommandBuffer CommandPool::BeginSingleTimeCommand()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void CommandPool::EndSingleTimeCommand(VkCommandBuffer commandBuffer, VkQueue graphicsQueue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
	}
}

