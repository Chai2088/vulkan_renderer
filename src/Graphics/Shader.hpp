#pragma once

#include <vector>

namespace VulkanRenderer
{
	class ShaderProgram
	{
	public:
		void LoadShader(const char* vertPath, const char* fragPath);
		void CreateShaderModule(const std::vector<char>& code);
	private:
		
	};
}