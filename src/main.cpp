// vulkan_renderer.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "Window.hpp"
#include "Graphics/Renderer.hpp"
#include "Core/Engine.hpp"
#include "Tests/TestScene.hpp"
#include "Tests/TestShadow.hpp"
int main()
{
    VulkanRenderer::Engine* engine = VulkanRenderer::Engine::GetInstance();
	VulkanRenderer::TestScene scene;
	
	engine->Initialize();
	scene.OnCreate();
	scene.Initialize();
	engine->SetScene(&scene);

	engine->Run();

	scene.Shutdown();
	engine->Shutdown();
	return 0;
}
