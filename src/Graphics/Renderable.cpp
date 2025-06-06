#include "Core/Engine.hpp"
#include "Core/Utils.hpp"
#include "Scene/GameObject.hpp"
#include "Transform/TransformComponent.hpp"
#include "PushConstants.hpp"
#include "Renderable.hpp"
namespace
{
	const uint32_t MAX_BINDLESS_TEXTURES = 1000;
}


namespace VulkanRenderer
{
	Renderable::Renderable() : mModel(nullptr)
	{
		//Registers the component in the factory and assigns a name and id
		Engine::GetInstance()->GetFactory().Register<Renderable>();
		mId = Engine::GetInstance()->AssignComponentId<Renderable>();
		mName = Engine::GetInstance()->AssignComponentName<Renderable>();
	}
	void Renderable::OnCreate()
	{
		//Add the renderable to the system
		Engine::GetInstance()->GetRenderer().AddToSystem(this);
	}
	void Renderable::Initialize()
	{

	}
	void Renderable::Shutdown()
	{

	}
	//This must be called before begin renderpass ->option11: 1 renderpass per model
	
	void Renderable::PreDraw(CommandBuffer& commandBuffer, VkDescriptorSet& textureDescriptor, VkDevice device, void* uniformBufferMapped)
	{
		//Update textures
		std::vector<VkDescriptorImageInfo> imageInfos(MAX_BINDLESS_TEXTURES);
		ResourceManager& rm = Engine::GetInstance()->GetResourceManager();
		std::unordered_map<std::string, int> readTextures;

		uint32_t idx = 0;
		for (uint32_t i = 0; i < mModel->mMats.size(); ++i)
		{
			Material* mat = mModel->mMats[i];
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
			memcpy(static_cast<uint8_t*>(uniformBufferMapped) + sizeof(UniformBufferObject) + sizeof(MaterialData) * i, &mat->mData, sizeof(MaterialData));
		}
		//Update only if there is any change in the descriptor
		if (idx > 0)
		{
			//Write the descriptor
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = textureDescriptor;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWrite.descriptorCount = idx;
			descriptorWrite.pImageInfo = imageInfos.data();

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}
	void Renderable::Draw(CommandBuffer& commandBuffer, GraphicsPipeline& pipeline)
	{
		//mModel->Draw(commandBuffer);
		//Draw all the meshes from the model
		for (uint32_t i = 0; i < mModel->mMeshes.size(); ++i)
		{
			Mesh* mesh = mModel->mMeshes[i];

			//Update the push constant
			PushConstants pushConstant;
			pushConstant.model = GetOwner()->GetTransformComponent()->GetWorldTransform();
			pushConstant.matIdx = mesh->mMatIdx;
			commandBuffer.BindPushConstants(pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, &pushConstant);

			//Bind the vertex buffer
			std::vector<VkBuffer> vertexBuffers = { mesh->mVertexBuffer };
			std::vector<VkDeviceSize> offsets = { 0 }; //Offsets are from where it starts reading each buffer
			commandBuffer.BindVertexBuffer(vertexBuffers, offsets, 0);
			
			//Bind the index buffer
			commandBuffer.BindIndexBuffer(mesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			//Record draw command
			commandBuffer.DrawIndexed(mesh->mIndexCount, 1, 0, 0, 0);
		}
	}
}