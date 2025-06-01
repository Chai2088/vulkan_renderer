#include <glm/glm.hpp>

struct GLFWwindow;
namespace VulkanRenderer
{
	class Camera
	{
	public:
		Camera();
		Camera(glm::vec3 pos, glm::vec3 dir);
		
		glm::mat4 GetViewMatrix() const;
		void OffsetCamera(glm::vec3 offset);

		glm::vec3 GetDirection() const;
		glm::vec3 GetPosition() const;
		glm::vec3 GetUpVector() const;

		void SetPosition(glm::vec3 pos);
		void SetDirection(glm::vec3 dir);

		void ProcessMouseInput(double xPos, double yPos, bool mouseDown);
		
		void EditorDebug();
	private:
		glm::vec3 mPos;
		glm::vec3 mFront;
		glm::vec3 mUp;

		float pitch;
		float yaw;
	};
}