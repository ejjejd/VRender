#include "camera.h"

namespace graphics
{
	void Camera::ComputeBasis()
	{
		ForwardAxis.x = glm::cos(Yaw) * glm::cos(Pitch);
		ForwardAxis.y = glm::sin(Pitch);
		ForwardAxis.z = glm::sin(Yaw) * glm::cos(Pitch);

		UpAxis = glm::vec3(0.0f, -1.0f, 0.0f);
		RightAxis = glm::normalize(glm::cross(UpAxis, ForwardAxis));
		UpAxis = glm::cross(ForwardAxis, RightAxis);
	}

	void Camera::Move(const CameraMoveDirection direction, const float deltaTime)
	{
		glm::vec3 offset;

		switch (direction)
		{
		case CameraMoveDirection::Forward: 
			{
				offset = ForwardAxis * Speed;
			} break; 
		case CameraMoveDirection::Backward:
			{
				offset = -ForwardAxis * Speed;
			} break;
		case CameraMoveDirection::Right:
			{
				offset = RightAxis * Speed;
			} break;
		case CameraMoveDirection::Left:
			{
				offset = -RightAxis * Speed;
			} break;
		case CameraMoveDirection::Up:
			{
				offset = UpAxis * Speed;
			} break;
		case CameraMoveDirection::Down:
			{
				offset = -UpAxis * Speed;
			} break;
		}

		Position += offset * deltaTime;
	}
}