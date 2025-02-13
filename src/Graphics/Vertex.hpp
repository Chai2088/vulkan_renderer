#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include <array>

namespace VulkanRenderer
{
    struct InstanceData
    {
        glm::mat4 model;
    };
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static std::array<VkVertexInputBindingDescription, 2> getBindingDescription()
        {
            std::array<VkVertexInputBindingDescription, 2> bindingDescription = {};

            bindingDescription[0].binding = 0;
            bindingDescription[0].stride = sizeof(Vertex);
            bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            bindingDescription[1].binding = 1;
            bindingDescription[1].stride = sizeof(InstanceData);
            bindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

            //Position attribute
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            //Color attribute
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            //Texture coordinate attribute
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            for (int i = 0; i < 4; ++i)
            {
                attributeDescriptions[3 + i].binding = 1;
                attributeDescriptions[3 + i].location = 3 + i;  // Locations 3,4,5,6
                attributeDescriptions[3 + i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attributeDescriptions[3 + i].offset = offsetof(InstanceData, model) + sizeof(glm::vec4) * i;
            }

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const 
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };
}

template<> struct std::hash<VulkanRenderer::Vertex> 
{
    size_t operator()(VulkanRenderer::Vertex const& vertex) const
    {
        return ((std::hash<glm::vec3>()(vertex.pos) ^
            (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            (std::hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};