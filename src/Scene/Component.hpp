#pragma once
#include <string>
#include <cstdint>
namespace VulkanRenderer
{
	class Component
	{
	public:
		Component() {}
		virtual void OnCreate() {};
		virtual void Initialize() {};
		virtual void Shutdown() {};

		uint32_t GetId() const { return mId; }
		std::string GetName() const { return mName; }
		bool IsActive() { return mActive; }
	protected:
		uint32_t mId;
		std::string mName;
		bool mActive = true;
	};
}