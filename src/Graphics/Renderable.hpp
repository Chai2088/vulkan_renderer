#pragma once
#include "Scene/Component.hpp"
#include "GraphicsPipeline.hpp"
#include "CommandBuffer.hpp"
#include "RenderResources.hpp"
#include "Material.hpp"

namespace VulkanRenderer
{
	class Renderable : public Component
	{
	public:
		Renderable();
		void OnCreate() override;
		void Initialize() override;
		void Shutdown() override;
		void Edit() override;
		//Submit textures and materials to the shader
		//Update the texture indices for draw
		void PreDraw(CommandBuffer& commandBuffer, VkDescriptorSet& textureDescriptor, VkDevice device, void* uniformBufferMapped);
		//Draw the model
		void Draw(CommandBuffer& commandBuffer, GraphicsPipeline& pipeline);
	
		Model* mModel;
		bool mIsVisible;
		int32_t mTexIndex;
		glm::vec3 mColor; //For debug purposes
	};
}