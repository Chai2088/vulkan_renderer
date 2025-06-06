#pragma once
#include <string>
#include <cstdint>


namespace VulkanRenderer
{
	class GameObject;
	class Component
	{
	public:
		Component() {}
		virtual void OnCreate() {};
		virtual void Initialize() {};
		virtual void Shutdown() {};

		//ImGUI edit
		virtual void Edit() {};

		uint32_t GetId() const { return mId; }
		std::string GetName() const { return mName; }
		bool IsActive() { return mActive; }
		GameObject* GetOwner();
		void SetOwner(GameObject* obj);

	protected:
		uint32_t mId;
		std::string mName;
		bool mActive = true;

		GameObject* mOwner = nullptr;
	};
}