#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <limits>
#include <chrono>

#include "Core/Engine.hpp"
#include "Window.hpp"
#include "Vertex.hpp"
#include "PushConstants.hpp"
#include "CommandBuffer.hpp"
#include "Material.hpp"
#include "Scene/GameObject.hpp"
#include "Transform/TransformComponent.hpp"
#include "Renderer.hpp"

#undef min
#undef max

namespace
{
	//Information for the model
	std::vector<VulkanRenderer::Vertex> vertices;
	std::vector<uint32_t> indices;

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	const std::string MODEL_PATH = "data/Models/viking_room.obj";
	const std::string TEXTURE_PATH = "data/Textures/viking_room.png";
	const std::string OTHER_TEXTURE_PATH = "data/Textures/Chai_Calvo.jpg";

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_EXT_descriptor_indexing"
	};

	const int MAX_FRAMES_IN_FLIGHT = 2;
	const uint32_t MAX_BINDLESS_TEXTURES = 1000;
	const uint32_t MAX_LIGHTS = 10;
	const uint32_t MAX_MATERIALS = 100;

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		auto message_string = pCallbackData->pMessage;
		int icon = MB_ICONWARNING;
		const char* title = "Info";

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			icon = MB_ICONINFORMATION;
			title = "Info";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			icon = MB_ICONINFORMATION;
			title = "Info";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			icon = MB_ICONWARNING;
			title = "Warning";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			icon = MB_ICONERROR;
			title = "Error";
			break;
		default:
			break;
		}
		//Display the pop up message
		MessageBox(NULL, message_string, title, icon);

		return VK_FALSE;
	}


	int RateDevice(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		//Give score to the available gpus
		int score = 0;
		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
		{
			score += 1000;
		}
		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		if (!deviceFeatures.geometryShader) 
		{
			return 0;
		}
		return score;
	}

	std::string LoadShader(const char* path)
	{
		std::ifstream file(path, std::ios::binary);

		if (!file.good()) 
		{
			throw std::runtime_error("failed to open file!");
		}

		std::stringstream shaderCodeStream;
		shaderCodeStream << file.rdbuf();
		file.close();

		return shaderCodeStream.str();
	}

	//Helper function to load the model
	//void LoadMeshData(std::vector<VulkanRenderer::Vertex>& vertices, std::vector<uint32_t>& indices, const char* meshPath)
	//{
	//	tinyobj::attrib_t attrib;
	//	std::vector<tinyobj::shape_t> shapes;
	//	std::vector<tinyobj::material_t> materials;
	//	std::string warn, err;

	//	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, meshPath)) {
	//		throw std::runtime_error(warn + err);
	//	}
	//	
	//	std::unordered_map<VulkanRenderer::Vertex, uint32_t> uniqueVertices;
	//	//each shape is a mesh and all the meshes/shapes together creates the model
	//	for (const auto& shape : shapes) {
	//		for (const auto& index : shape.mesh.indices) {
	//			VulkanRenderer::Vertex vertex{};
	//			
	//			vertex.pos = {
	//				attrib.vertices[3 * index.vertex_index + 0],
	//				attrib.vertices[3 * index.vertex_index + 1],
	//				attrib.vertices[3 * index.vertex_index + 2]
	//			};
	//			vertex.normal = {
	//				attrib.normals[3 * index.normal_index + 0],
	//				attrib.normals[3 * index.normal_index + 1],
	//				attrib.normals[3 * index.normal_index + 2]
	//			};

	//			vertex.texCoord = {
	//				attrib.texcoords[2 * index.texcoord_index + 0],
	//				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	//			};
	//			
	//			vertex.color = { 1.0f, 1.0f, 1.0f };

	//			if (uniqueVertices.count(vertex) == 0) {
	//				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
	//				vertices.push_back(vertex);
	//			}

	//			indices.push_back(uniqueVertices[vertex]);
	//		}
	//	}
	//}

	uint32_t counter = 0;
	float lastFrame = 0.0f;
}

namespace VulkanRenderer
{
	void Renderer::InitVulkan(Window* window)
	{
		mWindow = window;		
		CreateInstance();
		SetupDebugMessenger();
		CreateWindowSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateDescriptorPools();
		CreateShadowPipeline();
		CreateGraphicsPipeline();
		CreateCommandPool();
		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();
		//CreateTextureImage();
		CreateTextureSampler();
		//LoadMeshData(vertices, indices, MODEL_PATH.c_str());
		//CreateVertexBuffer(vertices, mVertexBuffer, mVertexBufferMemory);
		//CreateIndexBuffer(indices, mIndexBuffer, mIndexBufferMemory);
		//CreateDummyRenderable();
		CreateInstanceBuffer();
		CreateUniformBuffers();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSyncObjects();
	}
	
	void Renderer::Update()
	{
		//Get window handle
		GLFWwindow* window = mWindow->GetHandle();
		
		//Compute delta time
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		Camera& cam = Engine::GetInstance()->GetCamera();

		//Update camera position
		const float cameraSpeed = 500.0f * deltaTime; // adjust accordingly
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cam.OffsetCamera(cameraSpeed * cam.GetDirection());
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cam.OffsetCamera(-cameraSpeed * cam.GetDirection());
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cam.OffsetCamera(-glm::normalize(glm::cross(cam.GetDirection(), cam.GetUpVector())) * cameraSpeed);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cam.OffsetCamera(glm::normalize(glm::cross(cam.GetDirection(), cam.GetUpVector())) * cameraSpeed);

		//ImGui Light Debug
		for (uint32_t i = 0; i < mLights.size(); ++i)
		{
			std::string stId = "Light" + std::to_string(i);
			ImGui::PushID(stId.c_str());
			ImGui::Text(stId.c_str());
			mLights[i]->Edit();
			ImGui::PopID();
		}
	}

	std::unordered_map<Model*, std::vector<InstanceData>> Renderer::PrepareDraw()
	{
		//Update textures
		std::vector<VkDescriptorImageInfo> imageInfos(MAX_BINDLESS_TEXTURES);
		ResourceManager& rm = Engine::GetInstance()->GetResourceManager();
		std::unordered_map<std::string, int> readTextures;
		//Store this for the draw in next draw call
		std::unordered_map<Model*, std::vector<InstanceData>> readModels;
		
		uint32_t idx = 0;
		uint32_t matCount = 0;
		
		for (uint32_t j = 0; j < mRenderables.size(); ++j)
		{
			Model* model = mRenderables[j]->mModel;
			//store the transform of the model
			InstanceData data;
			data.model = mRenderables[j]->GetOwner()->GetTransformComponent()->GetWorldTransform();
			data.color = mRenderables[j]->mColor;
			readModels[model].push_back(data);

			//If the model was already read then we dont need to submit its material again
			if (readModels.find(model) != readModels.end())
			{
				continue;
			}
			//Fill the material buffers
			for (uint32_t i = 0; i < model->mMats.size(); ++i)
			{
				Material* mat = model->mMats[i];
				//TODO:material store the texture pointer directly instead of the name
				if (!mat->mAmbientTexName.empty())
				{
					//Check if the texture was already loaded to the buffer
					if (readTextures.find(mat->mAmbientTexName) != readTextures.end())
					{
						mat->mData.mAmbientTexIdx = readTextures[mat->mAmbientTexName];
					}
					else
					{
						imageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						Texture* diff = rm.GetResource<Texture>(mat->mAmbientTexName.c_str());
						imageInfos[idx].imageView = diff->mTextureImageView;
						mat->mData.mAmbientTexIdx = idx;
						readTextures[mat->mAmbientTexName] = idx;
						idx++;
					}
				}
				if (!mat->mDiffuseTexName.empty())
				{
					//Check if the texture was already loaded to the buffer
					if (readTextures.find(mat->mDiffuseTexName) != readTextures.end())
					{
						mat->mData.mDiffuseTexIdx = readTextures[mat->mDiffuseTexName];
					}
					else
					{
						imageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						Texture* diff = rm.GetResource<Texture>(mat->mDiffuseTexName.c_str());
						imageInfos[idx].imageView = diff->mTextureImageView;
						mat->mData.mDiffuseTexIdx = idx;
						readTextures[mat->mDiffuseTexName] = idx;
						idx++;
					}
				}
				if (!mat->mSpecularTexName.empty())
				{
					//Check if the texture was already loaded to the buffer
					if (readTextures.find(mat->mSpecularTexName) != readTextures.end())
					{
						mat->mData.mSpecularTexIdx = readTextures[mat->mSpecularTexName];
					}
					else
					{
						imageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						Texture* diff = rm.GetResource<Texture>(mat->mSpecularTexName.c_str());
						imageInfos[idx].imageView = diff->mTextureImageView;
						mat->mData.mSpecularTexIdx = idx;
						readTextures[mat->mSpecularTexName] = idx;
						idx++;
					}
				}

				//Copy the material data to the UBO
				memcpy(static_cast<uint8_t*>(mUniformBuffersMapped[mCurrentFrame]) + sizeof(UniformBufferObject) + sizeof(MaterialData) * matCount, &mat->mData, sizeof(MaterialData));
				matCount++;
			}
		}	
		//Update the instance buffer per each model
		for (auto& m : readModels)
		{
			m.first->UpdateInstanceBuffer(mDevice, m.second);
			m.first->mInstanceCount = m.second.size();
		}
		//Update only if there is any change in the descriptor
		if (idx > 0)
		{
			//Write the descriptor
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mDescriptorSets[4];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWrite.descriptorCount = idx;
			descriptorWrite.pImageInfo = imageInfos.data();

			vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
		}
		return readModels;
	}
	
	void Renderer::DrawFrame()
	{
		counter++;
		//wait for the previous frame
		vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		//Aquire the image from the swapchain
		uint32_t imageIndex;

		//Check if it needs to recreate the swapchain
		VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to recreate the swapchain");
		}

		//Reset
		vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
		//Clear the command buffer
		CommandBuffer& commandBuffer = mCommandPool.GetCommandBuffer(mCurrentFrame);

		//Populate the command buffer with (draw calls)
		RecordCommandBuffer(commandBuffer, imageIndex);
		
		//Update the UBO
		UpdateUniformBuffer(mCurrentFrame);
		//Update the lights
		UpdateLights(mCurrentFrame);

		//Prepare for submitting the command buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//semaphore to check if the image is available
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.GetCommandBuffer();

		//Semaphore to check if it finished rendering
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		//Check if it needs to recreate the swapchain
		result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mFramebufferResized)
		{
			mWindow->mFramebufferResized = false;
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present buffer!");
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	
	void Renderer::Shutdown()
	{
		Engine* engine = Engine::GetInstance();
		for (uint32_t i = 0; i < mRenderables.size(); i++)
		{
			mRenderables.at(i)->Shutdown();
			engine->GetFactory().Delete(mRenderables.at(i));
		}
	}
	
	void Renderer::ShutdownVulkan()
	{
		//Wait for all the processes to end
		vkDeviceWaitIdle(mDevice);

		CleanUpSwapChain();

		//Destroy texture sampler
		vkDestroySampler(mDevice, mTextureSampler, nullptr);
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
			vkFreeMemory(mDevice, mUniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

		for (uint32_t i = 0; i < mDescriptorSetLayouts.size(); ++i)
		{
			vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayouts[i], nullptr);
		}

		vkDestroyBuffer(mDevice, mInstanceBuffer, nullptr);
		vkFreeMemory(mDevice, mInstanceBufferMemory, nullptr);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
		}
		mPipeline.Shutdown();
		mShadowPipeline.Shutdown();
		mCommandPool.Shutdown();
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
		
		vkDestroyDevice(mDevice, nullptr);

		if (enableValidationLayers) 
		{
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}

	VkInstance Renderer::GetInstance()
	{
		return mInstance;
	}

	VkPhysicalDevice Renderer::GetPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	VkDevice Renderer::GetDevice()
	{
		return mDevice;
	}

	VkQueue Renderer::GetGraphicsQueue()
	{
		return mGraphicsQueue;
	}

	VkRenderPass Renderer::GetRenderPass()
	{
		return mRenderPass;
	}

	VkSampleCountFlagBits Renderer::GetMSAASampleCount()
	{
		return mMsaaSamples;
	}

	void Renderer::WaitIdle()
	{
		//Wait for all the processes to end
		vkDeviceWaitIdle(mDevice);
	}

	Mesh* Renderer::LoadMesh(const char* meshPath)
	{
		//Mesh* mesh = Engine::GetInstance()->GetFactory().Create<Mesh>();
		//std::vector<Vertex> vertices{};
		//std::vector<uint32_t> indices{};

		////Load mesh vertex and index data
		//LoadMeshData(vertices, indices, meshPath);
		//mesh->mIndexCount = indices.size();

		////Create vulkan buffers
		//CreateVertexBuffer(vertices, mesh->mVertexBuffer, mesh->mVertexBufferMemory);
		//CreateIndexBuffer(indices, mesh->mIndexBuffer, mesh->mIndexBufferMemory);

		//return mesh;
		return nullptr;
	}

	void Renderer::LoadMaterial(const char* matPath)
	{
		//std::map<std::string, int> materialMap;
		//std::vector<tinyobj::material_t> mats;
		//std::string err, war;
		//std::ifstream file(matPath);
		//if (!file.good())
		//	return;
		////Load the materials
		//tinyobj::LoadMtl(&materialMap, &mats, &file, &war, &err);
		////Loop all the materials in the file
		//for (auto& mat : mats)
		//{
		//	//Load all the textures from the material
		//	std::string prefix = "data/Textures/";
		//	LoadTexture((prefix + mat.ambient_texname).c_str()); //Load ambient texture
		//	LoadTexture((prefix + mat.diffuse_texname).c_str()); //Load diffuse texture
		//	LoadTexture((prefix + mat.specular_texname).c_str()); //Load specular texture

		//	//TODO: load more textures if required
		//}
	}

	Texture* Renderer::LoadTexture(const char* texPath)
	{
		int textWidth;
		int textHeight;
		int textChannels;
		stbi_uc* pixels = stbi_load(texPath, &textWidth, &textHeight, &textChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = textWidth * textHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("Could not load image");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		//Create a staging buffer
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		//Map the image data to the staging buffer
		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(mDevice, stagingBufferMemory);

		Texture* texture = Engine::GetInstance()->GetFactory().Create<Texture>();
		texture->mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textWidth, textHeight)))) + 1;
		texture->mWidth = textWidth;
		texture->mHeight = textHeight;
		texture->mTexturePath = texPath;

		//Create the image
		CreateImage(textWidth, textHeight, texture->mMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->mTextureImage, texture->mTextureImageMemory);

		//Transition image to TRANSFER_DST_OPTIMAL format
		TransitionImageLayout(texture->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->mMipLevels);

		//Copy the buffer to the image
		CopyBufferToImage(stagingBuffer, texture->mTextureImage, static_cast<uint32_t>(textWidth), static_cast<uint32_t>(textHeight));

		//Generate Mipmaps for the texture
		GenerateMipmaps(texture->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, textWidth, textHeight, texture->mMipLevels);

		//Create the image view
		texture->mTextureImageView = CreateImageView(texture->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture->mMipLevels);

		//Clean up
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
		
		return texture;
	}

	void Renderer::DestroyMesh(Mesh* mesh)
	{
		//Delete the vertex buffer
		vkDestroyBuffer(mDevice, mesh->mVertexBuffer, nullptr);
		vkFreeMemory(mDevice, mesh->mVertexBufferMemory, nullptr);

		//Delete the index buffer
		vkDestroyBuffer(mDevice, mesh->mIndexBuffer, nullptr);
		vkFreeMemory(mDevice, mesh->mIndexBufferMemory, nullptr);
	}

	void Renderer::DestroyTexture(Texture* tex)
	{
		//Delete the image buffer
		vkDestroyImage(mDevice, tex->mTextureImage, nullptr);
		vkFreeMemory(mDevice, tex->mTextureImageMemory, nullptr);

		//Delete the image view
		vkDestroyImageView(mDevice, tex->mTextureImageView, nullptr);
	}

	void Renderer::AddToSystem(Renderable* rend)
	{
		mRenderables.push_back(rend);
	}

	void Renderer::RemoveFromSystem(Renderable* rend)
	{
		auto it = std::find(mRenderables.begin(), mRenderables.end(), rend);
		mRenderables.erase(it);
	}

	void Renderer::AddLight(Light* light)
	{
		mLights.push_back(light);
	}

	void Renderer::RemoveLight(Light* light)
	{
		auto it = std::find(mLights.begin(), mLights.end(), light);
		mLights.erase(it);
	}
	
	//Vulkan Init functions
	void Renderer::CreateWindowSurface()
	{
		if (glfwCreateWindowSurface(mInstance, mWindow->GetHandle(), nullptr, &mSurface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}

	}
	
	void Renderer::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures2 deviceFeatures{};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;
		deviceFeatures.features.sampleRateShading = VK_TRUE;

		// Enable descriptor indexing features
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
		descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		descriptorIndexingFeatures.pNext = nullptr;

		// Link feature chain
		deviceFeatures.pNext = &descriptorIndexingFeatures;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pNext = &deviceFeatures;

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
	}
	
	void Renderer::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.minImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
		mSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;
	}
	
	void Renderer::CreateImageViews()
	{
		mSwapChainImageViews.resize(mSwapChainImages.size());
	
		for (size_t i = 0; i < mSwapChainImages.size(); i++)
		{
			mSwapChainImageViews[i] = CreateImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}
	
	void Renderer::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		for (const auto& device : devices) 
		{
			if (IsDeviceSuitable(device)) 
			{
				mPhysicalDevice = device;
				mMsaaSamples = GetMaxUsableSampleCount();
				break;
			}
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	
	void Renderer::CreateGraphicsPipeline()
	{
		mPipeline.Initialize(mDevice);

		//Create the push constant range
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.size = sizeof(PushConstants);
		pushConstantRange.offset = 0;

		std::vector<VkPushConstantRange> pushConstants{pushConstantRange};

		//Create the pipeline layout and the pipeline
		mPipeline.CreatePipelineLayout(mDescriptorSetLayouts, pushConstants);
		mPipeline.CreatePipeline(mRenderPass, pushConstantRange, mMsaaSamples);
	}

	void Renderer::CreateShadowPipeline()
	{
		mShadowPipeline.Initialize(mDevice);
		mShadowPipeline.PrepareFrameBuffer();
		mShadowPipeline.CreateUniformBuffer();
		mShadowPipeline.SetDescriptorSets(mDescriptorPool);
		mShadowPipeline.CreatePipelineLayout();
		mShadowPipeline.CreatePipeline();
	}
	
	void Renderer::CreateRenderPass()
	{
		//Attachment descriptions
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = mSwapChainImageFormat;
		colorAttachment.samples = mMsaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = mMsaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = mSwapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//Attachment references
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}
	
	void Renderer::CreateFrameBuffers()
	{
		mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) 
		{
			std::array<VkImageView, 3> attachments = {
				mColorImageView,
				mDepthImageView,
				mSwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = mSwapChainExtent.width;
			framebufferInfo.height = mSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	
	void Renderer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);
		mCommandPool.Initialize(mDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndices.graphicsFamily.value());
	}
	
	void Renderer::CreateCommandBuffers()
	{
		mCommandPool.AllocateBuffers(MAX_FRAMES_IN_FLIGHT);
	}
	
	void Renderer::CreateSyncObjects()
	{
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //does not in the first frame

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {

				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}
	
	void Renderer::CreateVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	
		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void Renderer::CreateIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void Renderer::CreateInstanceBuffer()
	{
		VkDeviceSize bufferSize = sizeof(InstanceData) * mInstanceCount;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mInstanceBuffer,
			mInstanceBufferMemory);
	}

	void Renderer::CreateUniformBuffers()
	{
		VkDeviceSize minAlignment = GetMinAlignment();
		VkDeviceSize alignedSize = (sizeof(UniformBufferObject) + sizeof(MaterialData) * MAX_MATERIALS + minAlignment - 1) & ~(minAlignment - 1);
		VkDeviceSize bufferSize = alignedSize + sizeof(LightData) * MAX_LIGHTS;

		mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBuffersMemory[i]);

			vkMapMemory(mDevice, mUniformBuffersMemory[i], 0, bufferSize, 0, &mUniformBuffersMapped[i]);
		}
	}

	void Renderer::CreateDescriptorPools()
	{
		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(4 * MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_BINDLESS_TEXTURES);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(5 * MAX_FRAMES_IN_FLIGHT + 1); // Total 7 sets 
																				// 2 ubo, 2 matUbo, 2 lightUbo, 2, sampler, 1 texture
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT; // Needed for bindless

		if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	
	void Renderer::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		std::vector<VkDescriptorSetLayout> uboLayout(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayouts[0]);
		allocInfo.pSetLayouts = uboLayout.data();

		mDescriptorSets.resize(2 * MAX_FRAMES_IN_FLIGHT + 1);

		//Allocate the descriptor set for uniform buffers
		if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSets[0]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}


		//Allocate the descriptor set for texture sampler
		std::vector<VkDescriptorSetLayout> samplerLayout(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayouts[1]);
		allocInfo.pSetLayouts = samplerLayout.data();
		if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSets[MAX_FRAMES_IN_FLIGHT * 1]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		//Allocate the descriptor set for texture images (bindless texturing)
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayouts[2];
		if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSets[MAX_FRAMES_IN_FLIGHT * 2]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		mTexturesInUse.resize(MAX_FRAMES_IN_FLIGHT, nullptr);

		//Initialize the descriptor sets
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorBufferInfo matBufferInfo{};
			matBufferInfo.buffer = mUniformBuffers[i];
			matBufferInfo.offset = sizeof(UniformBufferObject);
			matBufferInfo.range = sizeof(MaterialData) * MAX_MATERIALS;

			size_t matSize = sizeof(MaterialData);
			size_t lightSize = sizeof(LightData);

			VkDeviceSize minAlignment = GetMinAlignment();
			VkDeviceSize alignedOffset = (sizeof(UniformBufferObject) + sizeof(MaterialData) * MAX_MATERIALS + minAlignment - 1) & ~(minAlignment - 1);

			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = mUniformBuffers[i];
			lightBufferInfo.offset = alignedOffset;
			lightBufferInfo.range = sizeof(LightData) * MAX_LIGHTS;

			VkDescriptorImageInfo samplerInfo{};
			samplerInfo.sampler = mTextureSampler;

			VkDescriptorImageInfo depthInfo = mShadowPipeline.GetDescriptorInfo();

			//Update the descriptor sets
			VkWriteDescriptorSet descriptorWrite{};
			std::array<VkWriteDescriptorSet, 4> uboDescriptorWrites{};

			//Update uniform buffer descriptor data
			uboDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrites[0].dstSet = mDescriptorSets[i];
			uboDescriptorWrites[0].dstBinding = 0;
			uboDescriptorWrites[0].dstArrayElement = 0;
			uboDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrites[0].descriptorCount = 1;
			uboDescriptorWrites[0].pBufferInfo = &bufferInfo;

			uboDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrites[1].dstSet = mDescriptorSets[i];
			uboDescriptorWrites[1].dstBinding = 1;
			uboDescriptorWrites[1].dstArrayElement = 0;
			uboDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrites[1].descriptorCount = 1;
			uboDescriptorWrites[1].pBufferInfo = &matBufferInfo;

			uboDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrites[2].dstSet = mDescriptorSets[i];
			uboDescriptorWrites[2].dstBinding = 2;
			uboDescriptorWrites[2].dstArrayElement = 0;
			uboDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrites[2].descriptorCount = 1;
			uboDescriptorWrites[2].pBufferInfo = &lightBufferInfo;

			//Set shadow map descriptor
			uboDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrites[3].dstSet = mDescriptorSets[i];
			uboDescriptorWrites[3].dstBinding = 3;
			uboDescriptorWrites[3].dstArrayElement = 0;
			uboDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			uboDescriptorWrites[3].descriptorCount = 1;
			uboDescriptorWrites[3].pImageInfo = &depthInfo;
			vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(uboDescriptorWrites.size()), uboDescriptorWrites.data(), 0, nullptr);

			//Update texture sampler descriptor
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mDescriptorSets[i + 2];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &samplerInfo;
			vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
		}
	}
	
	void Renderer::CreateDescriptorSetLayout()
	{
		//Specify the binding of the descriptor set
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding materialUboLayoutBinding{};
		materialUboLayoutBinding.binding = 1;
		materialUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialUboLayoutBinding.descriptorCount = 1;
		materialUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Specify binding for light ubo
		VkDescriptorSetLayoutBinding lightUboLayoutBinding{};
		lightUboLayoutBinding.binding = 2;
		lightUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightUboLayoutBinding.descriptorCount = 1;
		lightUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Shadow map sampler binding layout
		VkDescriptorSetLayoutBinding shadowMapLayoutBinding{};
		shadowMapLayoutBinding.binding = 3;
		shadowMapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		shadowMapLayoutBinding.descriptorCount = 1;
		shadowMapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Specify texture sampler binding layout
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Specify texture binding layout
		VkDescriptorSetLayoutBinding textureLayoutBinding{};
		textureLayoutBinding.binding = 0;
		textureLayoutBinding.descriptorCount = MAX_BINDLESS_TEXTURES;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		textureLayoutBinding.pImmutableSamplers = nullptr;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Create the extension structure to allow partially bound descriptors
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = 1;  // We are modifying 1 binding (the texture array)
		VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
		bindingFlagsInfo.pBindingFlags = &flags;

		std::array<VkDescriptorSetLayoutBinding, 4> binding0 = { uboLayoutBinding, materialUboLayoutBinding, lightUboLayoutBinding, shadowMapLayoutBinding };
		std::array<VkDescriptorSetLayoutBinding, 1> binding1 = { samplerLayoutBinding };
		std::array<VkDescriptorSetLayoutBinding, 1> binding2 = { textureLayoutBinding };
		mDescriptorSetLayouts.resize(3);

		//Create the descriptor set
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(binding0.size());
		layoutInfo.pBindings = binding0.data();

		//Create ubo descriptor layout
		if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayouts[0]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		//Create sampler descriptor layout
		layoutInfo.bindingCount = static_cast<uint32_t>(binding1.size());
		layoutInfo.pBindings = binding1.data();
		if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayouts[1]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		//Create image descriptor layout
		layoutInfo.bindingCount = static_cast<uint32_t>(binding2.size());
		layoutInfo.pBindings = binding2.data();
		layoutInfo.pNext = &bindingFlagsInfo;
		if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayouts[2]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	
	void Renderer::RecreateSwapChain()
	{
		//When the window is minimized wait 
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->GetHandle(), &width, &height);
		while (width == 0 || height == 0) 
		{
			if (glfwWindowShouldClose(mWindow->GetHandle()))
				return;

			glfwGetFramebufferSize(mWindow->GetHandle(), &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(mDevice);

		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();
	}

	void Renderer::CleanUpSwapChain()
	{
		//Destroy MSAA buffer
		vkDestroyImageView(mDevice, mColorImageView, nullptr);
		vkDestroyImage(mDevice, mColorImage, nullptr);
		vkFreeMemory(mDevice, mColorImageMemory, nullptr);
		
		//Destroy DepthBuffer
		vkDestroyImageView(mDevice, mDepthImageView, nullptr);
		vkDestroyImage(mDevice, mDepthImage, nullptr);
		vkFreeMemory(mDevice, mDepthImageMemory, nullptr);

		for (size_t i = 0; i < mSwapChainFramebuffers.size(); i++) 
		{
			vkDestroyFramebuffer(mDevice, mSwapChainFramebuffers[i], nullptr);
		}

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) 
		{
			vkDestroyImageView(mDevice, mSwapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	}

	bool Renderer::CheckValidationSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
	
	void Renderer::SetupDebugMessenger()
	{
		if (!enableValidationLayers) 
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
	
	void Renderer::CreateInstance()
	{
		//validation
		if (enableValidationLayers && !CheckValidationSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		//for debug callbacks
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
		createInfo.ppEnabledExtensionNames = extensions.data();

		//include the validation layers in the instance info
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}
	
	void Renderer::RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		Camera& cam = Engine::GetInstance()->GetCamera();
		auto models = PrepareDraw();
		commandBuffer.BeginCommandBuffer();

		//Draw depth map
		VkRenderPassBeginInfo depthPassInfo = mShadowPipeline.GetRenderPassBeginInfo();
		commandBuffer.BeginRenderPass(depthPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		mShadowPipeline.SetupDraw(commandBuffer, mCurrentFrame);
		//Start recording all the shadow commands
		for (auto& m : models)
		{
			m.first->DrawShadow(commandBuffer, mShadowPipeline, m.first->mInstanceBuffer, m.first->mInstanceCount);
		}
		commandBuffer.EndRenderPass();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapChainExtent;

		//Clear values for color and depth-stencil
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		commandBuffer.BeginRenderPass(renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		commandBuffer.BindPipeline(mPipeline.GetPipeline());

		//Set the viewport and scissor size
		commandBuffer.SetViewport(0.0f, 0.0f, mSwapChainExtent, 0.0f, 1.0f, 0, 1);
		commandBuffer.SetScissor({ 0, 0 }, mSwapChainExtent, 0, 1);

		//TODO: descpriptor set stored inside the renderables
		//Bind the descriptor set 
		commandBuffer.BindDescriptorSet(mPipeline.GetPipelineLayout(), 0, 1, &mDescriptorSets[mCurrentFrame]);		//UBO DescriptorSet
		commandBuffer.BindDescriptorSet(mPipeline.GetPipelineLayout(), 1, 1, &mDescriptorSets[mCurrentFrame + 2]);	//Sampler DesciptorSet
		commandBuffer.BindDescriptorSet(mPipeline.GetPipelineLayout(), 2, 1, &mDescriptorSets[4]);					//BindlessTexture DescriptorSet

		//Start recording all the commands
		for (auto& m: models)
		{

			//UpdateMaterial(mRenderables.at(i)->mMaterial);

			//Temp: Create 2 model instances
			//glm::mat4 rotSca = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			//rotSca *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			//
			//glm::mat4 model0 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)) * rotSca;
			//glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * rotSca;

			//model submits render commands to the commandBuffer for each mesh it contains
			m.first->Draw(commandBuffer, mPipeline, m.first->mInstanceBuffer, m.first->mInstanceCount);

			////TODO: Update the vertex and index buffer per mesh in model and draw indexed
			////Bind the vertex buffer
			//std::vector<VkBuffer> vertexBuffers = { mRenderables[i]->mMesh->mVertexBuffer, mInstanceBuffer};
			//std::vector<VkDeviceSize> offsets = { 0 , 0 }; //Offsets are from where it starts reading each buffer
			//commandBuffer.BindVertexBuffer(vertexBuffers, offsets, 0);
			////Bind the index buffer
			//commandBuffer.BindIndexBuffer(mRenderables[i]->mMesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			//commandBuffer.DrawIndexed(mRenderables[i]->mMesh->mIndexCount, 1, 0, 0, 0);
		}
		//Render Imgui
		ImGui::Render();
		auto* renderData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.GetCommandBuffer());

		commandBuffer.EndRenderPass();
		commandBuffer.EndCommandBuffer();
	}

	QueueFamilyIndices Renderer::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
			
			if (presentSupport) {
				indices.presentFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}
	
	bool  Renderer::IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate &&supportedFeatures.samplerAnisotropy;
	}
	
	bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}


		return requiredExtensions.empty();
	}
	
	SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

		//Get the format count
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

		//If the format count is not zero populate the format array
		if (formatCount != 0) 
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
		}

		//Query the present modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
	
	VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}
	
	VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) 
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	
	VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<unsigned>::max()) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			int width, height;
			glfwGetFramebufferSize(mWindow->GetHandle(), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	
	VkShaderModule Renderer::CreateShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = 4 * code.size();
		createInfo.pCode = code.data();
		
		VkShaderModule shaderModule;

		if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}
	
	uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
	
	void Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		//Create buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create buffer!");
		}

		//Allocate the memory for the buffer
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		//Bind the memory to the buffer
		vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
	}
	
	void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
	{
		//Creates a single time command buffer
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		
		//Record copy buffer command
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void Renderer::DestroyBuffer(VkBuffer buffer, VkDeviceMemory memory)
	{
		vkDestroyBuffer(mDevice, buffer, nullptr);
		vkFreeMemory(mDevice, memory, nullptr);
	}
	
	void Renderer::CreateTextureImage()
	{

	}
	
	void Renderer::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//Bilinear filtering
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		//Texture set to repeat mode when u,v,w is outside [0, 1] 
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		//Enable anisotropy
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//If false then [0, 1], else [0, texWidth] and [0, texHeight]
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 12.0f;
	
		if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	
	void Renderer::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();
		//Create the depth image
		CreateImage(mSwapChainExtent.width, mSwapChainExtent.height, 1, mMsaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);
		//Transition to depth stencil layout
		TransitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

		mDepthImageView = CreateImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	}

	void Renderer::UpdateUniformBuffer(uint32_t currentImage)
	{
		Camera& cam = Engine::GetInstance()->GetCamera();
		UniformBufferObject ubo{};
	
		ubo.view = cam.GetViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), mSwapChainExtent.width / (float)mSwapChainExtent.height, 0.1f, 5000.0f);
		ubo.proj[1][1] *= -1;

		ubo.lightCount = mLights.size();
		ubo.viewPos = cam.GetPosition();

		//Update ubo buffer for the shadow
		Light* dirLight = nullptr;
		for (auto& l : mLights)
		{
			if (l->mData.mType == 1)
			{
				dirLight = l;
				break;
			}
		}
		if (dirLight)
		{
			ubo.depthMVP = mShadowPipeline.UpdateUniformBuffer(dirLight, currentImage);
		}
		else
			ubo.depthMVP = glm::mat4(1.0f);
		memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void Renderer::UpdateInstanceBuffer(const std::vector<InstanceData>& instances)
	{
		VkDeviceSize bufferSize = sizeof(InstanceData) * instances.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// Copy data to staging buffer
		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, instances.data(), bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);
		
		//Copy staging buffer to the instance buffer
		CopyBuffer(stagingBuffer, mInstanceBuffer, bufferSize);

		//Free memory
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void Renderer::CreateDummyRenderable()
	{
		//Renderable* renderable = Engine::GetInstance()->GetFactory().Create<Renderable>();
		//renderable->mMesh = new Mesh();
		////renderable->mMaterial = new Material();
		////renderable->mMaterial->mDiffuseTexture = new Texture();

		//renderable->mMesh->mVertexBuffer = mVertexBuffer;
		//renderable->mMesh->mVertexBufferMemory = mVertexBufferMemory;
		//renderable->mMesh->mIndexBuffer = mIndexBuffer;
		//renderable->mMesh->mIndexBufferMemory = mIndexBufferMemory;

		//mRenderables.push_back(renderable);
	}

	void Renderer::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevel, VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevel;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(mDevice, image, imageMemory, 0);
	}

	void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevel)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevel;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;
		
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndSingleTimeCommands(commandBuffer);
	}

	VkImageView Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		//Create the image view
		VkImageView imageView;
		if (vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkSampler Renderer::CreateSampler(VkFilter filter, VkSamplerMipmapMode mipMapMode, VkSamplerAddressMode addressMode, float loadBias, float anisotropy, float minLod, float maxLod, VkBorderColor borderColor, VkCompareOp compareOp)
	{
		VkSampler sampler;
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.mipmapMode = mipMapMode;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.mipLodBias = loadBias;
		samplerInfo.maxAnisotropy = anisotropy;
		samplerInfo.minLod = minLod;
		samplerInfo.maxLod = maxLod;
		samplerInfo.borderColor = borderColor;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = compareOp;

		if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("Failde to create a sampler");
		return sampler;
	}

	VkFramebuffer Renderer::CreateFramebuffer(VkRenderPass renderPass, VkImageView* attachments, int32_t attachmentCount, uint32_t width, uint32_t height)
	{
		VkFramebuffer framebuffer;
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentCount;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a framebuffer");
		return framebuffer;
	}

	VkCommandBuffer Renderer::BeginSingleTimeCommands()
	{
		return mCommandPool.BeginSingleTimeCommand();
	}

	void Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		mCommandPool.EndSingleTimeCommand(commandBuffer, mGraphicsQueue);
	}

	VkFormat Renderer::FindSupportedFormats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat Renderer::FindDepthFormat()
	{
		return FindSupportedFormats(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool Renderer::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void Renderer::GenerateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &formatProperties);

		//Check support for linear filtering features
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) 
		{
			//Transition from image destination layout to src layout
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;
		
			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			//Transition from image transfer layout to shader read layout
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			//Divide the mip dimensions for the next level
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		//Transfer from trasfer dst optimal to shader read optimal as this is never blitted in the loop
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer);
	}

	VkSampleCountFlagBits Renderer::GetMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		else if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		else if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		else if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		else if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		else if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void Renderer::CreateColorResources()
	{
		VkFormat colorFormat = mSwapChainImageFormat;

		CreateImage(mSwapChainExtent.width, mSwapChainExtent.height, 1, mMsaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT 
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mColorImage, mColorImageMemory);
		mColorImageView = CreateImageView(mColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void Renderer::UpdateTexture()
	{
		//std::vector<VkDescriptorImageInfo> imageInfos(MAX_BINDLESS_TEXTURES);
		//ResourceManager& rm = Engine::GetInstance()->GetResourceManager();
		////TODO: renderables can share textures so get all the texture references
		//uint32_t j = 0;
		//for (uint32_t i = 0; i < mRenderables.size(); ++i)
		//{
		//	//Check if the renderable has a material
		//	if (mRenderables[i]->mMats.empty())
		//	{
		//		mRenderables[i]->mTexIndex = -1;
		//	}
		//	else
		//	{
		//		for (uint32_t j = 0; j < mRenderables[i]->mMats.size(); ++j)
		//		{
		//			//Stores the textures in an array to prepare for rendering
		//			imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//			//TODO: Load all the textures of the material to the renderer
		//			Texture* diff = rm.GetResource<Texture>(mRenderables[i]->mMats[j]->mDiffuseTexName.c_str());
		//			imageInfos[j].imageView = diff->mTextureImageView;
		//			mRenderables[i]->mTexIndex = i;
		//		}
		//	}
		//}

		//VkWriteDescriptorSet descriptorWrite{};
		//descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = mDescriptorSets[4];
		//descriptorWrite.dstBinding = 0;
		//descriptorWrite.dstArrayElement = 0;
		//descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		//descriptorWrite.descriptorCount = j;
		//descriptorWrite.pImageInfo = imageInfos.data();

		//vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}

	void Renderer::UpdateMaterial(Material* material)
	{
		//Sanity check
		if (material == nullptr)
			return;
		memcpy(static_cast<uint8_t*>(mUniformBuffersMapped[mCurrentFrame]) + sizeof(UniformBufferObject), &material->mData, sizeof(MaterialData));
	}

	void Renderer::UpdateLights(uint32_t currentImage)
	{
		VkDeviceSize minAlignment = GetMinAlignment();
		VkDeviceSize alignedOffset = (sizeof(UniformBufferObject) + sizeof(MaterialData) * MAX_MATERIALS + minAlignment - 1) & ~(minAlignment - 1);

		for (uint32_t i = 0; i < mLights.size(); ++i)
		{
			size_t offset = alignedOffset + i * sizeof(LightData);
			memcpy(static_cast<uint8_t*>(mUniformBuffersMapped[currentImage]) + offset, &mLights.at(i)->mData, sizeof(LightData));
		}
	}

	VkDeviceSize Renderer::GetMinAlignment()
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
		VkDeviceSize minAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		return minAlignment;
	}

	std::vector<uint32_t> Renderer::CompileShaderToSPIRV(const std::string& shaderSource, const char* shaderName, shaderc_shader_kind shaderType)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Enable optimization
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		// preprocess
		shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(shaderSource, shaderType, shaderName, options);

		if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			throw std::runtime_error(preprocessed.GetErrorMessage());
		}

		std::string shaderString = { preprocessed.cbegin(), preprocessed.cend() };

		// Compile GLSL to SPIR-V
		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString, shaderType, shaderName, options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			throw std::runtime_error(module.GetErrorMessage());
		}

		return { module.cbegin(), module.cend() };
	}
}