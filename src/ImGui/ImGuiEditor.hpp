#pragma once
#include <vulkan/vulkan.hpp>
namespace VulkanRenderer
{
	class Window;
	class Editor
	{
	public:
		void Initialize(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, 
						VkQueue graphicsQueue, VkRenderPass renderPass, VkSampleCountFlagBits msaaCount,
						Window* window);
		void BeginFrame();
		void EndFrame();
		void Shutdown();
	protected:
		void CreateDescriptorPool();
	private:
		VkDevice			mDevice;
		VkDescriptorPool	mDescriptorPool;
	};
}