#pragma once
#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>

#include <vector>

#include "CommandBuffer.hpp"
namespace VulkanRenderer
{
	class Light;
	struct FrameBufferAttachment
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView imageView;
	};

	class ShadowPipeline
	{
	public:
		ShadowPipeline();
		ShadowPipeline(const VkDevice& device);
		void Initialize(const VkDevice& device);
		void Shutdown();

		void PrepareFrameBuffer();
		void CreateRenderPass();
		void CreateUniformBuffer();
		void SetDescriptorSets(VkDescriptorPool descriptorPool);
		void CreatePipeline();
		void CreatePipelineLayout();

		const VkPipelineLayout& GetPipelineLayout() const;
		const VkPipeline& GetPipeline() const;
		VkRenderPassBeginInfo GetRenderPassBeginInfo();
		VkDescriptorImageInfo GetDescriptorInfo();
		void SetupDraw(CommandBuffer& commandBuffer, int32_t currentFrame);
		glm::mat4 UpdateUniformBuffer(Light* light, uint32_t currentFrame);

	protected:

	private:
		VkDevice							mDevice;
		VkPipeline							mShadowPipeline;
		VkPipelineLayout					mPipelineLayout;
		VkFramebuffer						mFrameBuffer;
		FrameBufferAttachment				mAttachments;
		VkSampler							mDepthSampler;
		VkDescriptorImageInfo				mDescriptor;
		VkRenderPass						mRenderPass;
		int32_t								mWidth, mHeight;

		std::vector<VkDescriptorSetLayout>  mDescriptorLayouts;
		std::vector<VkDescriptorSet>		mDescriptorSets;

		std::vector<VkBuffer>				mUniformBuffers;	
		std::vector<VkDeviceMemory>			mUniformBufferMemory;	
		std::vector<void*>					mUniformBufferMapped;	
	};
}