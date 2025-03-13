#pragma once
#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>

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
		VkShaderModule CreateShaderModule(const char * filename, const char* name, shaderc_shader_kind shaderType);
		void CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& layouts, 
								  std::vector<VkPushConstantRange>& pushConstantRanges);
		
		const VkPipelineLayout& GetPipelineLayout() const;
		const VkPipeline& GetPipeline() const;

	protected:
		std::vector<uint32_t> CompileShaderToSPIRV(const std::string& shaderSource, 
												   const char* shaderName, shaderc_shader_kind shaderType);
	private:
		VkDevice							mDevice;
		VkPipeline							mGraphicsPipeline;
		VkPipelineLayout					mPipelineLayout;



		std::vector<VkDescriptorSetLayout>  mDescriptorLayouts;
	};
}