
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include <iostream>

#include "Camera.hpp"

namespace
{
	bool firstMouse = true;
	double lastX;
	double lastY;

	double offsetX;
	double offsetY;
}
namespace VulkanRenderer
{
	Camera::Camera()
	{
		//mPos = glm::vec3(0.0f, 0.0f, 3.0f);
		
		//Test the camera looking from the light
		mPos = glm::vec3(300.0f, 300.0f, 3.0f);
		mFront = glm::vec3(1.0f, -1.0f, 1.0f);
		mUp = glm::vec3(0.0f, 1.0f, 0.0f);

		yaw = -90.0f;
		pitch = 0.0f;
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
	void Camera::ProcessMouseInput(double xPos, double yPos, bool mouseDown)
	{
		//Update the last position and the gets the offset
		if (firstMouse)
		{
			lastX = xPos;
			lastY = yPos;
			firstMouse = false;
		}

		offsetX = xPos - lastX;
		offsetY = lastY - yPos;
		lastX = xPos;
		lastY = yPos;
		//if the mouse button is not clicked then stop here and dont move the camera
		if (!mouseDown)
			return;
		float sensitivity = 0.1f;
		offsetX *= sensitivity;
		offsetY *= sensitivity;
		
		yaw += offsetX;
		pitch += offsetY;

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
		ImGui::DragFloat("Offset x:", (float*)&offsetX);
		ImGui::DragFloat("Offset y:", (float*)&offsetY);
	}
}
