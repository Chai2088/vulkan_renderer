#include <vulkan/vulkan.hpp>

namespace VulkanRenderer
{
	class Renderer
	{
	public:
		void InitVulkan();
		void ShutdownVulkan();

	private:
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkPhysicalDevice mPhysicalDevice;


		bool CheckValidationSupport();
		void SetupDebugMessenger();
		void CreateInstance();
		void PickPhysicalDevice();
	};
}