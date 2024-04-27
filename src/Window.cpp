#include <vulkan/vulkan.hpp>
#include "Window.hpp"

namespace VulkanRenderer
{
	void Window::InitWindow()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(640, 480, "Vulkan Renderer", NULL, NULL);
	}
}

