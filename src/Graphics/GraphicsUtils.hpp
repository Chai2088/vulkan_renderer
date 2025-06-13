#pragma once

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>

namespace VulkanRenderer
{
	std::string LoadShader(const char* filename);
	std::vector<uint32_t> CompileShaderToSPIRV(const std::string& shaderSource, const char* shaderName, shaderc_shader_kind shaderType);
	VkShaderModule CreateShaderModule(VkDevice device, const char* filename, const char* name, shaderc_shader_kind shaderType);
}