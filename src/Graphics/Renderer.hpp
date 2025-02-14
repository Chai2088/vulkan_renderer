#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <optional>
#include <unordered_map>

#include "Vertex.hpp"
#include "RenderResources.hpp"
#include "Renderable.hpp"
#include "Camera.hpp"

namespace VulkanRenderer
{
	class Window;

	struct UniformBufferObject
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

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
		bool Update();
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
		VkDescriptorSetLayout mDescriptorSetLayout;
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
		VkBuffer mInstanceBuffer;
		VkDeviceMemory mInstanceBufferMemory;
		uint32_t mInstanceCount = 10;

		//Descriptors
		VkDescriptorPool mDescriptorPool;
		std::vector<VkDescriptorSet> mDescriptorSets;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		//Textures
		VkSampler mTextureSampler;
		std::vector<std::string> mCurrentTexturePaths;
		std::unordered_map<std::string, Texture*> mTextures;

		//Depth Buffer
		VkImage mDepthImage;
		VkDeviceMemory mDepthImageMemory;
		VkImageView mDepthImageView;

		//Antialising
		VkSampleCountFlagBits mMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
		
		//Msaa buffers
		VkImage mColorImage;
		VkDeviceMemory mColorImageMemory;
		VkImageView mColorImageView;

		std::vector<Renderable*> mRenderables;

		bool CheckValidationSupport();
		void SetupDebugMessenger();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateWindowSurface();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateInstanceBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPools();
		void CreateDescriptorSets();
		//Textures
		void CreateTextureImage();
		void CreateTextureSampler();
		//Depth buffer
		void CreateDepthResources();

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
		
		//Create buffer helper function for staging buffers
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
		
		//Create image helper function
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevel, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevel);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void LoadTexture(const std::string& texturePath);

		//Helper function to create an Image View
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevel);

		//Helper function to record command buffers
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBUffer);

		//Helper function to find supported formats
		VkFormat FindSupportedFormats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		//Helper function to generate mipmaps
		void GenerateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		//Helper function to obtain maximum sample count per pixel
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		//Creates MSAA buffer
		void CreateColorResources();

		//UBO
		void UpdateUniformBuffer(uint32_t currentImage);
		
		//Updates the instance buffer
		void UpdateInstanceBuffer(const std::vector<InstanceData>& data);

		//Temp check renderable is working
		void CreateDummyRenderable();
	};
}