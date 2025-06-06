
#include <tiny_obj_loader.h>

#include "Core/Engine.hpp"
#include "PushConstants.hpp"
#include "RenderResources.hpp"
namespace
{
	//Helper function to help set the material data from tinyobj material
	void SetMaterial(VulkanRenderer::Material* mat, tinyobj::material_t& matData)
	{
		mat->mData.mAmbient = glm::vec3(matData.ambient[0], matData.ambient[1], matData.ambient[2]);
		mat->mData.mDiffuse = glm::vec3(matData.diffuse[0], matData.diffuse[1], matData.diffuse[2]);
		mat->mData.mSpecular = glm::vec3(matData.specular[0], matData.specular[1], matData.specular[2]);
		mat->mData.mIllum = matData.illum;
		mat->mData.mIor = matData.ior;
		mat->mData.mShininess = matData.shininess;

		if(!matData.ambient_texname.empty())
			mat->mAmbientTexName = "data/" + matData.ambient_texname;
		if(!matData.diffuse_texname.empty())
			mat->mDiffuseTexName = "data/" + matData.diffuse_texname;
		if(!matData.specular_texname.empty())
			mat->mSpecularTexName = "data/" + matData.specular_texname;
	}
}

namespace VulkanRenderer
{
	Model::Model()
	{
		Renderer& r = Engine::GetInstance()->GetRenderer();
		VkDeviceSize bufferSize = sizeof(InstanceData) * 10;

		r.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mInstanceBuffer,
			mInstanceBufferMemory);
	}
	void Model::UpdateInstanceBuffer(VkDevice device, const std::vector<InstanceData>& instances)
	{
		Renderer& r = Engine::GetInstance()->GetRenderer();
		VkDeviceSize bufferSize = sizeof(InstanceData) * instances.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		r.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// Copy data to staging buffer
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, instances.data(), bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		//Copy staging buffer to the instance buffer
		r.CopyBuffer(stagingBuffer, mInstanceBuffer, bufferSize);

		//Free memory
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	void Model::LoadModel(const char* filename)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, "data/Materials/")) {
			throw std::runtime_error(warn + err);
		}
		//Loop all the materials and create the material resource
		for (auto& m : materials)
		{
			Material* mat = Engine::GetInstance()->GetFactory().Create<Material>();
			SetMaterial(mat, m);
			mMats.push_back(mat);
		}
		//Loop all the shapes and create a mesh item
		for (auto& s : shapes)
		{
			Mesh* mesh = Engine::GetInstance()->GetFactory().Create<Mesh>();
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<Vertex, uint32_t> uniqueVertices;
			//Get the vertex and indices for the mesh
			for (const auto& index : s.mesh.indices) {
				VulkanRenderer::Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };
				//Avoid duplicate vertices using indices
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
			//Assign the material to the mesh
			mesh->mMatIdx = s.mesh.material_ids[0];
			mesh->mIndexCount = indices.size();

			//Create vulkan buffers
			Engine::GetInstance()->GetRenderer().CreateVertexBuffer(vertices, mesh->mVertexBuffer, mesh->mVertexBufferMemory);
			Engine::GetInstance()->GetRenderer().CreateIndexBuffer(indices, mesh->mIndexBuffer, mesh->mIndexBufferMemory);

			mMeshes.push_back(mesh);
		}
	}
	void Model::Draw(CommandBuffer& commandBuffer, GraphicsPipeline& pipeline, VkBuffer instanceBuffer, uint32_t instances)
	{
		for (uint32_t i = 0; i < mMeshes.size(); ++i)
		{
			Mesh* mesh = mMeshes[i];

			//Update the push constant
			PushConstants pushConstant{};
			pushConstant.matIdx = mesh->mMatIdx;
			commandBuffer.BindPushConstants(pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, &pushConstant);

			//Bind the vertex buffer
			std::vector<VkBuffer> vertexBuffers = { mesh->mVertexBuffer , instanceBuffer};
			std::vector<VkDeviceSize> offsets = { 0, 0 }; //Offsets are from where it starts reading each buffer
			commandBuffer.BindVertexBuffer(vertexBuffers, offsets, 0);

			//Bind the index buffer
			commandBuffer.BindIndexBuffer(mesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			//Record draw command
			commandBuffer.DrawIndexed(mesh->mIndexCount, instances, 0, 0, 0);
		}
	}
}
