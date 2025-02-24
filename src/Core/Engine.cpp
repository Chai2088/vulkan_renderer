
#include "Engine.hpp"

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
	}
	void Engine::Update()
	{
		mRenderer.Update();
	}
	void Engine::Draw()
	{
		mRenderer.DrawFrame();
	}
	void Engine::Shutdown()
	{
		//Shutdown all the systems
		mResourceManager.Shutdown();
		mFactory.Shutdown();
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

			mRenderer.Update();
			mRenderer.DrawFrame();

			glfwPollEvents();
		}
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
}