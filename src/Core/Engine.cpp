#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include "Scene/Scene.hpp"
#include "Engine.hpp"
namespace
{
	void MouseCallback(GLFWwindow* window, double xPos, double yPos)
	{
		VulkanRenderer::Camera& cam = VulkanRenderer::Engine::GetInstance()->GetCamera();
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse)
			cam.ProcessMouseInput(xPos, yPos, glfwGetMouseButton(window, 0));
	}

}
namespace VulkanRenderer
{
	Engine* Engine::mEngine = nullptr;

	Engine* Engine::GetInstance()
	{
		if (mEngine == nullptr)
			mEngine = new Engine();
		return mEngine;
	}
	void Engine::Initialize()
	{
		mWindow.InitWindow();
		mRenderer.InitVulkan(&mWindow);
		mResourceManager.Initialize();
		mEditor.Initialize(mRenderer.GetInstance(), mRenderer.GetPhysicalDevice(), mRenderer.GetDevice(),
						   mRenderer.GetGraphicsQueue(), mRenderer.GetRenderPass(), mRenderer.GetMSAASampleCount(),
						   &mWindow);
		//Camera Init
		//glfwSetInputMode(mWindow->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(mWindow.GetHandle(), MouseCallback);
	}
	void Engine::Update()
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		mEditor.BeginFrame();
		mCurrentScene->UpdateAllTransforms();
		mRenderer.Update(); 
		//Output framerate for debug reasons
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		ImGui::Text("Framerate: %f", 1.0f / time);
		
		ImGuiIO& io = ImGui::GetIO();
		double mouse_x, mouse_y;
		glfwGetCursorPos(mWindow.GetHandle(), &mouse_x, &mouse_y);
		io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		ImGui::Text("Mouse Pos: %.2f, %.2f\n", io.MousePos.x, io.MousePos.y);
		ImGui::Text("Mouse Down: %d\n", io.MouseDown[0]);
		ImGui::Text("Capture mouse %d", io.WantCaptureMouse);
		mCamera.EditorDebug();
	}
	void Engine::Draw()
	{
		mEditor.EndFrame();
		mRenderer.DrawFrame();
	}
	void Engine::Shutdown()
	{
		//Shutdown all the systems, order matters
		mResourceManager.Shutdown();
		mFactory.Shutdown();
		mEditor.Shutdown();
		mWindow.Shutdown();
		mRenderer.ShutdownVulkan();

		//Delete the singleton
		delete mEngine;
	}
	void Engine::Run()
	{
		while (!glfwWindowShouldClose(mWindow.GetHandle()))
		{
			//Check if ESC has been pressed to exit
			if (glfwGetKey(mWindow.GetHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				break;

			Update();
			Draw();

			glfwPollEvents();
		}
	}
	void Engine::SetScene(Scene* scene)
	{
		mCurrentScene = scene;
	}
	Factory& Engine::GetFactory()
	{
		return mFactory;
	}
	Renderer& Engine::GetRenderer()
	{
		return mRenderer;
	}
	ResourceManager& Engine::GetResourceManager()
	{
		return mResourceManager;
	}
	Camera& Engine::GetCamera()
	{
		return mCamera;
	}
}