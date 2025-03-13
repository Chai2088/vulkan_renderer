#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Scene/Component.hpp"
namespace VulkanRenderer
{
	struct Transform
	{
		Transform();
		Transform(const Transform& other);
		glm::vec3 mPosition;
		glm::vec3 mRotation;
		glm::vec3 mScale;
	};
	class TransformComponent : public Component
	{
	public:
		TransformComponent();
		void Initialize() override;
		void UpdateTransforms();
		void UpdatePosition();
		void UpdateRotation();
		void UpdateScale();

		void SetLocalPosition(glm::vec3 pos);
		void SetLocalRotation(glm::vec3 rot);
		void SetLocalScale(glm::vec3 sca);

		glm::vec3 GetWorldPosition();
		glm::vec3 GetWorldRotation();
		glm::vec3 GetWorldScale();

		glm::mat4 GetWorldTransform();

	private:
		TransformComponent* mParentTransform;
		Transform mLocal;
		Transform mWorld;
	};
}