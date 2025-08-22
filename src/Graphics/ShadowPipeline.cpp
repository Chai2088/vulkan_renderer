#include "Core/Engine.hpp"
#include "GraphicsUtils.hpp"
#include "Light.hpp"
#include "ShadowPipeline.hpp"

namespace
{
	const VkFormat offscreenDepthFormat{ VK_FORMAT_D16_UNORM };
	constexpr int32_t shadowMapSize = 2048;
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	float depthBiasConstant = 1.25f;
	// Slope depth bias factor, applied depending on polygon's slope
	float depthBiasSlope = 1.75f;

	//Uniform buffer for shadow map
	struct ShadowUBO
	{
		glm::mat4 depthMVP;
	};

}

namespace VulkanRenderer
{
	ShadowPipeline::ShadowPipeline()
	{
	}
	ShadowPipeline::ShadowPipeline(const VkDevice& device)
	{
		mDevice = device;
	}
	void ShadowPipeline::Initialize(const VkDevice& device)
	{
		mDevice = device;
	}
	void ShadowPipeline::Shutdown()
	{
		Renderer& r = Engine::GetInstance()->GetRenderer();
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			r.DestroyBuffer(mUniformBuffers[i], mUniformBufferMemory[i]);
		}
		//Delete the image buffer
		vkDestroyImage(mDevice, mAttachments.image, nullptr);
		vkFreeMemory(mDevice, mAttachments.memory, nullptr);
		vkDestroyImageView(mDevice, mAttachments.imageView, nullptr);

		vkDestroyPipeline(mDevice, mShadowPipeline, nullptr);
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		
		vkDestroySampler(mDevice, mDepthSampler, nullptr);
		
		vkDestroyDescriptorSetLayout(mDevice, mDescriptorLayouts[0], nullptr);
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
		
		vkDestroyFramebuffer(mDevice, mFrameBuffer, nullptr);
	}
	void ShadowPipeline::CreateRenderPass()
	{
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = offscreenDepthFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;													// No color attachments
		subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create depth render pass");
		}
	}
	void ShadowPipeline::CreateUniformBuffer()
	{
		Renderer& r = Engine::GetInstance()->GetRenderer();
		VkDeviceSize bufferSize = sizeof(ShadowUBO);

		mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		mUniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
		mUniformBufferMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			r.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBufferMemory[i]);

			vkMapMemory(mDevice, mUniformBufferMemory[i], 0, bufferSize, 0, &mUniformBufferMapped[i]);
		}
	}
	void ShadowPipeline::SetDescriptorSets(VkDescriptorPool descriptorPool)
	{
		//Create desciptorset layout
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		mDescriptorLayouts.resize(1);

		//DEPTH MAP BINDING IS IN THE SCENE GRAPHICS PIPELINE SIDE
		if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, mDescriptorLayouts.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the descriptor set layout");

		//Create the descriptor set for the UBO and write into the set
		std::vector<VkDescriptorSetLayout> uboLayout(MAX_FRAMES_IN_FLIGHT, mDescriptorLayouts[0]);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		allocInfo.pSetLayouts = uboLayout.data();
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		
		mDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate the descriptor sets");

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(ShadowUBO);

			//Update the descriptor sets
			VkWriteDescriptorSet descriptorWrite{};

			//Update uniform buffer descriptor data
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mDescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
		}

	}
	void ShadowPipeline::CreatePipeline()
	{
		//Prepares the framebuffer and creates the render pass
		auto vertShaderModule = CreateShaderModule(mDevice, "data/Shaders/shadow.vert", "vertShader", shaderc_glsl_vertex_shader);

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getSimpleAttributeDescriptions();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_TRUE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.minSampleShading = 0.2f;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 0;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_DEPTH_BIAS
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//Depth-stencil testing enabled
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 1;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;

		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mShadowPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shadow pipeline!");
		}

		vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);

	}
	void ShadowPipeline::PrepareFrameBuffer()
	{
		Renderer& r = Engine::GetInstance()->GetRenderer();

		mWidth = shadowMapSize;
		mHeight = shadowMapSize;

		// For shadow mapping we only need a depth attachment
		r.CreateImage(mWidth, mHeight, 1, VK_SAMPLE_COUNT_1_BIT, offscreenDepthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mAttachments.image, mAttachments.memory);

		//Create depth image view
		mAttachments.imageView = r.CreateImageView(mAttachments.image, offscreenDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		// Create sampler to sample from to depth attachment
		// Used to sample in the fragment shader for shadowed rendering		
		mDepthSampler = r.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, 1.0f, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_COMPARE_OP_LESS_OR_EQUAL);

		CreateRenderPass();

		// Create frame buffer
		mFrameBuffer = r.CreateFramebuffer(mRenderPass, &mAttachments.imageView, 1, mWidth, mHeight);
	}
	void ShadowPipeline::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorLayouts.size());
		pipelineLayoutInfo.pSetLayouts = mDescriptorLayouts.data();

		if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	const VkPipelineLayout& ShadowPipeline::GetPipelineLayout() const
	{
		return mPipelineLayout;
	}
	const VkPipeline& ShadowPipeline::GetPipeline() const
	{
		return mShadowPipeline;
	}

	VkRenderPassBeginInfo ShadowPipeline::GetRenderPassBeginInfo()
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFrameBuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = {(uint32_t)mWidth, (uint32_t)mHeight};

		//Clear values for color and depth-stencil
		VkClearValue clearValues{};
		clearValues.depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValues;
		return renderPassInfo;
	}

	VkDescriptorImageInfo ShadowPipeline::GetDescriptorInfo()
	{
		VkDescriptorImageInfo depthInfo{};
		depthInfo.sampler = mDepthSampler;
		depthInfo.imageView = mAttachments.imageView;
		depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		return depthInfo;
	}

	void ShadowPipeline::SetupDraw(CommandBuffer& commandBuffer, int32_t currentFrame)
	{
		commandBuffer.SetViewport(0.0f, 0.0f, { (uint32_t)mWidth, (uint32_t)mHeight }, 0.0f, 1.0f, 0, 1);
		commandBuffer.SetScissor({ 0, 0 }, { (uint32_t)mWidth, (uint32_t)mHeight }, 0, 1);
		//Depth bias
		commandBuffer.SetDepthBias(depthBiasConstant, depthBiasSlope);
		commandBuffer.BindPipeline(mShadowPipeline);
		commandBuffer.BindDescriptorSet(mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame]);		//UBO DescriptorSet
	}

	glm::mat4 ShadowPipeline::UpdateUniformBuffer(Light* light, uint32_t currentFrame)
	{
		float h = mWidth * 0.5f;
		glm::mat4 proj = glm::ortho(-h, h, -h, h, 1.0f, 1000.0f);
		proj[1][1] *= -1;
		glm::mat4 view = glm::lookAt(light->GetPosition(), light->GetPosition() + light->mData.mDirection, glm::vec3(0.0f, 1.0f, 0.0f));

		ShadowUBO ubo{};
		ubo.depthMVP = proj * view;
		memcpy(mUniformBufferMapped[currentFrame], &ubo, sizeof(ShadowUBO));
		return ubo.depthMVP;
	}

}