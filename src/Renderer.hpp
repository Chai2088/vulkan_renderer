#include <vulkan/vulkan.hpp>
#include <optional>

namespace VulkanRenderer
{
	class Window;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class Renderer
	{
	public:
		void InitVulkan(Window* window);
		void ShutdownVulkan();

	private:
		VkInstance mInstance;
		VkSurfaceKHR mSurface;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice mDevice;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		VkDebugUtilsMessengerEXT mDebugMessenger;


		bool CheckValidationSupport();
		void SetupDebugMessenger();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateWindowSurface(Window* window);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	};
}