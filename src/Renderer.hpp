#include <vulkan/vulkan.hpp>
#include <optional>

#include "Vertex.hpp"

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
	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Renderer
	{
	public:
		void InitVulkan(Window* window);
		void DrawFrame();
		void ShutdownVulkan();

	private:
		VkInstance mInstance;
		VkSurfaceKHR mSurface;
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;
		VkSwapchainKHR mSwapChain;
		VkFormat mSwapChainImageFormat;
		VkExtent2D mSwapChainExtent;
		VkRenderPass mRenderPass;
		VkPipelineLayout mPipelineLayout;
		VkPipeline mGraphicsPipeline;
		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandBuffers;

		//Syncronization
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;

		std::vector<VkImage> mSwapChainImages;
		std::vector<VkImageView> mSwapChainImageViews;
		std::vector<VkFramebuffer> mSwapChainFramebuffers;

		VkDebugUtilsMessengerEXT mDebugMessenger;
		Window* mWindow;

		uint32_t mCurrentFrame = 0;

		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;
		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;

		VkImageView mTextureImageView;

		bool CheckValidationSupport();
		void SetupDebugMessenger();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateWindowSurface();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		
		void RecreateSwapChain();
		void CleanUpSwapChain();


		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
		//Textures
		void CreateTextureImage();
		void CreateTextureImageView();

	};
}