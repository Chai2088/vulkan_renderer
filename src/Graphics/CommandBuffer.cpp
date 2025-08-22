#include "PushConstants.hpp"
#include "CommandBuffer.hpp"

namespace VulkanRenderer
{
	void CommandBuffer::Initialize(VkCommandBuffer commandBuffer)
	{
		mCommandBuffer = commandBuffer;
	}
	void CommandBuffer::BeginCommandBuffer()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}
	void CommandBuffer::EndCommandBuffer()
	{
		vkEndCommandBuffer(mCommandBuffer);
	}
	void CommandBuffer::ResetCommandBuffer()
	{
		vkResetCommandBuffer(mCommandBuffer, 0);
	}
	void CommandBuffer::BindVertexBuffer(std::vector<VkBuffer> buffers, std::vector<VkDeviceSize> offsets, uint32_t first)
	{
		vkCmdBindVertexBuffers(mCommandBuffer, first, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}
	void CommandBuffer::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type)
	{
		vkCmdBindIndexBuffer(mCommandBuffer, buffer, offset, type);
	}
	void CommandBuffer::BindDescriptorSet(VkPipelineLayout layout, uint32_t set, uint32_t count, const VkDescriptorSet* descriptorSets)
	{
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 
								count, descriptorSets, 0, nullptr);
	}
	void CommandBuffer::BindPushConstants(VkPipelineLayout layout, VkShaderStageFlags flags, uint32_t offset, PushConstants* pushConstant)
	{
		vkCmdPushConstants(mCommandBuffer, layout, flags, offset, sizeof(PushConstants), pushConstant);
	}
	void CommandBuffer::BindPipeline(VkPipeline pipeline)
	{
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	void CommandBuffer::BeginRenderPass(VkRenderPassBeginInfo renderPassInfo, VkSubpassContents contents)
	{
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, contents);
	}
	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(mCommandBuffer);
	}
	void CommandBuffer::SetViewport(float x, float y, VkExtent2D extent, float minDepth, float maxDepth, uint32_t first, uint32_t count)
	{
		//Set the viewport and scissor size
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = extent.width;
		viewport.height = extent.height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		vkCmdSetViewport(mCommandBuffer, first, count, &viewport);
	}
	void CommandBuffer::SetScissor(VkOffset2D offset, VkExtent2D extent, uint32_t first, uint32_t count)
	{
		VkRect2D scissor{};
		scissor.offset = offset;
		scissor.extent = extent;
		vkCmdSetScissor(mCommandBuffer, first, count, &scissor);
	}
	void CommandBuffer::SetDepthBias(float depthBiasConstant, float depthSlope)
	{
		vkCmdSetDepthBias(mCommandBuffer, depthBiasConstant, 0.0f, depthSlope);
	}
	void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	VkCommandBuffer& CommandBuffer::GetCommandBuffer()
	{
		return mCommandBuffer;
	}
}