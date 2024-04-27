#include <vulkan/vulkan.hpp>
#include "Window.hpp"

namespace VulkanRenderer
{
	void Window::InitWindow()
	{
		//Initialize the window 
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mWindow = glfwCreateWindow(640, 480, "Vulkan Renderer", NULL, NULL);
	}
	void Window::Shutdown()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}
	GLFWwindow* Window::GetHandle()
	{
		return mWindow;
	}
}

