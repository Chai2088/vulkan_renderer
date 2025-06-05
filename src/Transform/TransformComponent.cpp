#include <glm/gtc/matrix_transform.hpp>
#include "Scene/GameObject.hpp"
#include "TransformComponent.hpp"

namespace VulkanRenderer
{
	Transform::Transform() :
		mPosition(glm::vec3(0.0f)), mScale(glm::vec3(1.0f)), mRotation(glm::vec3(0.0f))
	{

	}
	Transform::Transform(const Transform& other)
	{
		mPosition = other.mPosition;
		mScale = other.mScale;
		mRotation = other.mRotation;
	}
	TransformComponent::TransformComponent() : mParentTransform(nullptr)
	{
		//Register, assign name and id
		Engine::GetInstance()->GetFactory().Register<TransformComponent>();
		mName = Engine::GetInstance()->AssignComponentName<TransformComponent>();
		mId = Engine::GetInstance()->AssignComponentId<TransformComponent>();
	}
	void TransformComponent::Initialize()
	{
		//mParentTransform = GetOwner()->GetTransformComponent();
	}
	void TransformComponent::UpdateTransforms()
	{
		if (mParentTransform == nullptr)
		{
			mWorld = mLocal;
		}
		else
		{
			//Update world coordinates
			mWorld.mPosition = (mParentTransform->mWorld.mScale * mLocal.mPosition) + mParentTransform->mWorld.mPosition;
			mWorld.mScale = mParentTransform->mWorld.mScale * mLocal.mScale;
			mWorld.mRotation = mParentTransform->mWorld.mRotation + mLocal.mRotation;
		}
	}
	void TransformComponent::UpdatePosition()
	{
		if (mParentTransform != nullptr)
		{
			mWorld.mPosition = (mParentTransform->mWorld.mScale * mLocal.mPosition) + mParentTransform->mWorld.mPosition;
		}
		else
		{
			mWorld.mPosition = mLocal.mPosition;
		}

	}
	void TransformComponent::UpdateRotation()
	{
		if (mParentTransform != nullptr)
		{
			mWorld.mScale = mParentTransform->mWorld.mScale * mLocal.mScale;
		}
		else
		{
			mWorld.mScale = mLocal.mScale;
		}
	}
	void TransformComponent::UpdateScale()
	{
		if (mParentTransform != nullptr)
		{
			mWorld.mRotation = mParentTransform->mWorld.mRotation + mLocal.mRotation;
		}
		else
		{
			mWorld.mRotation = mLocal.mRotation;
		}
	}
	glm::mat4 TransformComponent::GetWorldTransform()
	{
		//Compute the world matrix
		glm::mat4 world = glm::scale(glm::mat4(1.0f), mWorld.mScale);
		world = glm::rotate(world, mWorld.mRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, mWorld.mRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, mWorld.mRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		world = glm::translate(world, mWorld.mPosition);
		return world;
	}

	void TransformComponent::SetLocalPosition(glm::vec3 pos)
	{
		mLocal.mPosition = pos;
	}

	void TransformComponent::SetLocalRotation(glm::vec3 rot)
	{
		mLocal.mRotation = rot;
	}

	void TransformComponent::SetLocalScale(glm::vec3 sca)
	{
		mLocal.mScale = sca;
	}

	glm::vec3 TransformComponent::GetWorldPosition()
	{
		//Make sure the world position is updated
		UpdatePosition();
		return mWorld.mPosition;
	}

	glm::vec3 TransformComponent::GetWorldRotation()
	{
		//Make sure the world rotation is updated
		UpdateRotation();
		return mWorld.mRotation;
	}

	glm::vec3 TransformComponent::GetWorldScale()
	{
		//Make sure the world scale is updated
		UpdateScale();
		return mWorld.mScale;
	}
}

