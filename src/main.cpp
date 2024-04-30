// vulkan_renderer.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "Window.hpp"
#include "Renderer.hpp"

int main()
{
	VulkanRenderer::Window window;
    VulkanRenderer::Renderer renderer;
    window.InitWindow();
    renderer.InitVulkan();

    while (!glfwWindowShouldClose(window.GetHandle())) {
        glfwPollEvents();
    }

    renderer.ShutdownVulkan();
    window.Shutdown();

	return 0;
}
