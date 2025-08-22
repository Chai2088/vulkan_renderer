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


		void CreateImGUITextureDescriptor(VkImageView imageView, VkSampler sampler);
	protected:
		void CreateDescriptorPool();
		void CreateTextureLayout();
	private:
		VkDevice					mDevice;
		VkDescriptorPool			mDescriptorPool;
		VkDescriptorSet				mImguiTextureDescriptor = VK_NULL_HANDLE;
		VkDescriptorSetLayout		mImguiTextureDescriptorLayout;
	};
}