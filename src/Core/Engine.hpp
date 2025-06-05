#pragma once
#include "Graphics/Renderer.hpp"		  //Renderer
#include "Window.hpp"					  //Window
#include "Factory.hpp"					  //Factory
#include "Resources/ResourceManager.hpp"  //ResourceManager
#include "Utils.hpp"					  //ParseIdName
#include "ImGui/ImGuiEditor.hpp"		  //Editor

#include <unordered_map>
namespace VulkanRenderer
{
	class Scene;
	class Engine
	{
	public:
		static Engine* GetInstance();

		template<typename T>
		uint32_t AssignComponentId() 
		{
			std::string idName = ParseIDName(typeid(T).name());
			//Check if the component is registered
			if (mRegisteredIds.find(idName) == mRegisteredIds.end())
				mRegisteredIds[idName] = mRegisteredIds.size();

			return mRegisteredIds.at(idName);
		}
		template<typename T>
		std::string AssignComponentName()
		{
			return ParseIDName(typeid(T).name());
		}

		//System states
		void Initialize();
		void Update();
		void Draw();
		void Shutdown();

		//Updates all the states that are called every frame
		void Run();

		//Set the current scene
		void SetScene(Scene* scene);

		//Get the systems
		Factory& GetFactory();
		Renderer& GetRenderer();
		ResourceManager& GetResourceManager();
		Camera& GetCamera();
	protected:
		//Singleton
		Engine() {};
		Engine(const Engine&) = delete;
		void operator=(const Engine&) = delete;
	
		static Engine* mEngine;
	private:
		Window			mWindow;
		Factory			mFactory;
		ResourceManager mResourceManager;
		Renderer		mRenderer;
		Editor			mEditor;
		Camera			mCamera;
		Scene*			mCurrentScene;
		
		//Ids
		std::unordered_map<std::string, uint32_t> mRegisteredIds;
	};
}