#include <vulkan/vulkan.hpp>
#include "Window.hpp"
#include "Graphics/Renderer.hpp"

namespace
{
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
	{
		auto app = reinterpret_cast<VulkanRenderer::Window*>(glfwGetWindowUserPointer(window));
		app->mFramebufferResized = true;
	}
}


namespace VulkanRenderer
{
	void Window::InitWindow()
	{
		//Initialize the window 
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mWindow = glfwCreateWindow(1280, 760, "Vulkan Renderer", NULL, NULL);
		glfwSetWindowUserPointer(mWindow, this);
		glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
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

