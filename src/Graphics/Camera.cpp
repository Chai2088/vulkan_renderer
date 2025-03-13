
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "Camera.hpp"

namespace
{
	bool firstMouse = true;
	double lastX;
	double lastY;
}
namespace VulkanRenderer
{
	Camera::Camera()
	{
		mPos = glm::vec3(0.0f, 0.0f, 3.0f);
		mFront = glm::vec3(0.0f, 0.0f, -1.0f);
		mUp = glm::vec3(0.0f, 1.0f, 0.0f);

		yaw = -90.0f;
	}
	Camera::Camera(glm::vec3 pos, glm::vec3 dir) : mPos(pos), mFront(dir)
	{

	}
	glm::mat4 Camera::GetViewMatrix() const
	{
		return glm::lookAt(mPos, mPos + mFront, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	void Camera::OffsetCamera(glm::vec3 offset)
	{
		mPos += offset;
	}
	glm::vec3 Camera::GetDirection() const
	{
		return mFront;
	}
	glm::vec3 Camera::GetPosition() const
	{
		return mPos;
	}
	glm::vec3 Camera::GetUpVector() const
	{
		return mUp;
	}
	void Camera::SetPosition(glm::vec3 pos)
	{
		mPos = pos;
	}
	void Camera::SetDirection(glm::vec3 dir)
	{
		mFront = dir;
	}
	void Camera::ProcessMouseInput(double xPos, double yPos)
	{
		if (firstMouse)
		{
			lastX = xPos;
			lastY = yPos;
			firstMouse = false;
		}

		float xOffset = xPos - lastX;
		float yOffset = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		float sensitivity = 0.1f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;
		
		yaw += xOffset;
		pitch += yOffset;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		double radYaw = glm::radians(yaw);
		double radPitch = glm::radians(pitch);

		glm::vec3 dir;
		dir.x = glm::cos(radYaw) * glm::cos(radPitch);
		dir.y = glm::sin(radPitch);
		dir.z = glm::sin(radYaw) * glm::cos(radPitch);
		mFront = dir;
	}
	void Camera::EditorDebug()
	{
		ImGui::DragFloat3("Camera Pos", &mPos[0]);
		ImGui::DragFloat3("Camera Dir", &mFront[0]);
	}
}
