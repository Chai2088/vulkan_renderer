#pragma once
#include "Window.hpp"					  //Window
#include "Factory.hpp"					  //Factory
#include "Graphics/Renderer.hpp"		  //Renderer
#include "Resources/ResourceManager.hpp"  //ResourceManager
#include "Utils.hpp"					  //ParseIdName

#include <unordered_map>
namespace VulkanRenderer
{
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

		//System states
		void Initialize();
		void Update();
		void Draw();
		void Shutdown();

		//Updates all the states that are called every frame
		void Run();

		//Get the systems
		Factory& GetFactory();
		Renderer& GetRenderer();
	protected:
		//Singleton
		Engine() {}
		Engine(const Engine&) = delete;
		void operator=(const Engine&) = delete;
	
		static Engine* mEngine;
	private:
		Window			mWindow;
		Factory			mFactory;
		ResourceManager mResourceManager;
		Renderer		mRenderer;

		//Ids
		std::unordered_map<std::string, uint32_t> mRegisteredIds;
	};
}