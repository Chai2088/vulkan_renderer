#pragma once
namespace VulkanRenderer
{
	class Component
	{
	public:
		virtual void OnCreate() = 0;
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	private:
		//TODO id or name
	};
}