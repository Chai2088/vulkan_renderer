#pragma once
#include <glm/glm.hpp>
#include <cstdint>
namespace VulkanRenderer
{
	struct PushConstants
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec3 viewPos;
		int32_t texIndex;
	};
}