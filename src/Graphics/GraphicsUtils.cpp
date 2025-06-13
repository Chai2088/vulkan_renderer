#include <fstream>
#include <iostream>
#include <string>

#include "GraphicsUtils.hpp"
namespace VulkanRenderer
{
	std::string LoadShader(const char* filename)
	{
		std::ifstream file(filename, std::ios::binary);

		if (!file.good())
		{
			throw std::runtime_error("failed to open file!");
		}

		std::stringstream shaderCodeStream;
		shaderCodeStream << file.rdbuf();
		file.close();

		return shaderCodeStream.str();
	}
	std::vector<uint32_t> CompileShaderToSPIRV(const std::string& shaderSource, const char* shaderName, shaderc_shader_kind shaderType)
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
			std::string errorMsg = module.GetErrorMessage();
			std::cout << errorMsg << std::endl;
			throw std::runtime_error(errorMsg);
		}

		return { module.cbegin(), module.cend() };
	}
	VkShaderModule CreateShaderModule(VkDevice device, const char* filename, const char* name, shaderc_shader_kind shaderType)
	{
		auto shaderCode = LoadShader(filename);
		auto spirVCode = CompileShaderToSPIRV(shaderCode, name, shaderType);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = 4 * spirVCode.size();
		createInfo.pCode = spirVCode.data();

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}
}