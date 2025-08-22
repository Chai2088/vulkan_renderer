#pragma once
#include <vulkan/vulkan.hpp>
namespace VulkanRenderer
{
	struct PushConstants;
	class CommandBuffer
	{
	public:
		void Initialize(VkCommandBuffer commandBuffer);
		void BeginCommandBuffer();
		void EndCommandBuffer();
		void ResetCommandBuffer();

		void BindVertexBuffer(std::vector<VkBuffer> buffers, std::vector<VkDeviceSize> offsets, uint32_t first);
		void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type);
		void BindDescriptorSet(VkPipelineLayout layout, uint32_t set, uint32_t count, const VkDescriptorSet* descriptorSets);
		void BindPushConstants(VkPipelineLayout layout, VkShaderStageFlags flags, uint32_t offset, PushConstants* pushConstant);
		void BindPipeline(VkPipeline pipeline);
		
		void BeginRenderPass(VkRenderPassBeginInfo renderPassInfo, VkSubpassContents contents);
		void EndRenderPass();

		void SetViewport(float x, float y, VkExtent2D extent, float minDepth, float maxDepth, uint32_t first, uint32_t count);
		void SetScissor(VkOffset2D offset, VkExtent2D extent, uint32_t first, uint32_t count);
		void SetDepthBias(float depthBiasConstant, float depthSlope);
			
		
		
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	
		VkCommandBuffer& GetCommandBuffer();
	private:
		VkCommandBuffer mCommandBuffer;
	};
}