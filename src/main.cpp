// vulkan_renderer.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "Window.hpp"
#include "Graphics/Renderer.hpp"
#include "Core/Engine.hpp"

int main()
{
    VulkanRenderer::Engine* engine = VulkanRenderer::Engine::GetInstance();
	
	engine->Initialize();
	engine->Run();
	engine->Shutdown();

	return 0;
}
