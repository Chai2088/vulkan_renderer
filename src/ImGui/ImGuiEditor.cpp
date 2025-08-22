#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <iostream>

#include "Window.hpp"
#include "Core/Engine.hpp"
#include "ImGuiEditor.hpp"

namespace VulkanRenderer
{
	void Editor::Initialize(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, 
                            VkQueue graphicsQueue, VkRenderPass renderPass, VkSampleCountFlagBits msaaCount, 
                            Window* window)
	{
        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // Initialize ImGui for GLFW and Vulkan
        ImGui_ImplGlfw_InitForVulkan(window->GetHandle(), true);

        mDevice = logicalDevice;

        //Create the descriptor pool
        CreateDescriptorPool();

        // Initialize ImGui Vulkan backend
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = instance;
        initInfo.PhysicalDevice = physicalDevice;
        initInfo.Device = logicalDevice;
        initInfo.QueueFamily = 0;
        initInfo.Queue = graphicsQueue;
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = mDescriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = 2;
        initInfo.MSAASamples = msaaCount;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        initInfo.RenderPass = renderPass;
        ImGui_ImplVulkan_Init(&initInfo); // Render pass will be set later
        ImGui_ImplVulkan_CreateFontsTexture();

        //Create texture layout
        CreateTextureLayout();
        //Create shadow texture descriptor
        VkDescriptorImageInfo depthInfo = Engine::GetInstance()->GetRenderer().mShadowPipeline.GetDescriptorInfo();
        CreateImGUITextureDescriptor(depthInfo.imageView, depthInfo.sampler);
	}
    void Editor::BeginFrame()
    {
        // Start the ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Editor");

        //Debug shadow texture
        if (mImguiTextureDescriptor != VK_NULL_HANDLE) {
            // Convert your Vulkan descriptor set to ImTextureID
            ImTextureID textureId = (ImTextureID)mImguiTextureDescriptor;
            // Display the texture (adjust size as needed)
            ImGui::Image(textureId, ImVec2(512, 512));
        }
    }
    void Editor::EndFrame()
    {
        ImGui::End();
    }
	void Editor::Shutdown()
	{
        // Cleanup
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        //destroy the descriptor pool
        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
    }
    void Editor::CreateImGUITextureDescriptor(VkImageView imageView, VkSampler sampler)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool; // Use your existing pool
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &mImguiTextureDescriptorLayout; // You'll need to create this

        vkAllocateDescriptorSets(mDevice, &allocInfo, &mImguiTextureDescriptor);

        VkDescriptorImageInfo descImageInfo = {};
        descImageInfo.sampler = sampler;
        descImageInfo.imageView = imageView;
        descImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet writeDesc = {};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = mImguiTextureDescriptor;
        writeDesc.descriptorCount = 1;
        writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDesc.pImageInfo = &descImageInfo;
        writeDesc.dstBinding = 0;

        vkUpdateDescriptorSets(mDevice, 1, &writeDesc, 0, nullptr);
    }
    void Editor::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;

        if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create ImGui descriptor pool!");
        }
    }
    void Editor::CreateTextureLayout()
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;

        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        layoutInfo.pBindings = &binding;

        vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mImguiTextureDescriptorLayout);
    }
}