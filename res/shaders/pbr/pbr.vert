#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
	vec3 Camera;
} globalUbo;

layout(set = 1, binding = 0) uniform MeshUBO
{
	mat4 Transform;
} meshUbo;

layout(location = 0) out vec3 Normal;

layout(location = 1) out flat vec3 Camera;

void main()
{
	Normal = (transpose(inverse(meshUbo.Transform)) * vec4(normal, 1.0f)).xyz;
	Camera = globalUbo.Camera;

	gl_Position = globalUbo.ToClip * globalUbo.ToCamera * meshUbo.Transform * vec4(position, 1.0f);
}