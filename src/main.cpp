// vulkan_renderer.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "Window.hpp"

int main()
{
	//VulkanRenderer::Window window;
	//window.InitWindow();
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
	return 0;
}
