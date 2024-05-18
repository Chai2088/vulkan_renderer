#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

namespace VulkanRenderer
{
	class Window
	{
	public:
		void InitWindow();
		void Shutdown();

		GLFWwindow* GetHandle();

		bool mFramebufferResized = false;
	private:
		GLFWwindow* mWindow;
		
	};
}
