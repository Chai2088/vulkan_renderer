#pragma once
#include <vulkan/vulkan.hpp>

namespace VulkanRenderer
{

	class GraphicsPipeline
	{
	public:
		GraphicsPipeline();
		GraphicsPipeline(const VkDevice& device);
		void Initialize(const VkDevice& device);
		void Shutdown();

		void CreatePipeline(const VkRenderPass& renderPass, const VkPushConstantRange& pushConstantRange, VkSampleCountFlagBits msaaSamples);
		void CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& layouts, 
								  std::vector<VkPushConstantRange>& pushConstantRanges);
		
		const VkPipelineLayout& GetPipelineLayout() const;
		const VkPipeline& GetPipeline() const;

	protected:
	private:
		VkDevice							mDevice;
		VkPipeline							mGraphicsPipeline;
		VkPipelineLayout					mPipelineLayout;



		std::vector<VkDescriptorSetLayout>  mDescriptorLayouts;
	};
}