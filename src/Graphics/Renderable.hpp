#pragma once
#include "Scene/Component.hpp"
#include "RenderResources.hpp"
#include "Material.hpp"

namespace VulkanRenderer
{
	class Renderable : public Component
	{
	public:
		Renderable();
		void OnCreate() override;
		void Initialize() override;
		void Shutdown() override;
	

		Mesh* mMesh;
		Material* mMaterial;
		bool mIsVisible;
		int32_t mTexIndex;
	};
}