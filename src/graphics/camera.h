#pragma once
#include "vrender.h"

namespace graphics
{
	enum class CameraMoveDirection
	{
		Forward,
		Backward,
		Right,
		Left,
		Up,
		Down
	};

	class Camera
	{
	private:
		glm::vec3 ForwardAxis;
		glm::vec3 RightAxis;
		glm::vec3 UpAxis;

		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;

		float OrthoWidth = 1280;

		bool Perspective = true;
		
		void ComputeBasis();
	public:
		float Yaw = glm::pi<float>() / 2.0f;
		float Pitch = 0.0f;
		float Fov = 45.0f;
		float AspectRatio = 1.77f;
		float Speed = 10.0f;
		glm::vec3 Position;

		inline void SetupAsPerspective(const glm::vec3& position, const float fov, const float ar, const float speed,
									   const float nearZ, const float farZ)
		{
			Position = position;

			Fov = fov;
			AspectRatio = ar;
			NearPlane = nearZ;
			FarPlane = farZ;

			Speed = speed;

			Perspective = true;

			ComputeBasis();
		}

		inline void SetupAsOrtho(const glm::vec3& position, const float width, const float speed)
		{
			Position = position;

			OrthoWidth = width;

			Speed = speed;

			Perspective = false;

			ComputeBasis();
		}

		void Move(const CameraMoveDirection direction, const float deltaTime);

		inline void AddRotation(const float yaw, const float pitch, const float deltaTime)
		{
			Yaw += yaw * deltaTime;
			Pitch += pitch * deltaTime;

			ComputeBasis();
		}

		inline glm::mat4 GetViewMatrix() const
		{
			return glm::lookAt(Position, Position + ForwardAxis, -UpAxis);
		}

		inline glm::mat4 GetProjection() const
		{
			if (Perspective)
				return glm::perspective(Fov, AspectRatio, NearPlane, FarPlane);

			float height = OrthoWidth / AspectRatio;
			return glm::ortho(-OrthoWidth / 2, OrthoWidth / 2, -height / 2, height / 2);
		}
	};
}