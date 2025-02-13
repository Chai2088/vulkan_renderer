#pragma once
#include "Component.hpp"

#include <cstdint> 
#include <unordered_map>
#include <string>
namespace VulkanRenderer
{
	class GameObejct
	{
	public:
		void OnCreate();
		void Initialize();
		void Shutdown();

	private:
		uint32_t mId;
		std::unordered_map<std::string, Component*> mComponents;
	};
}