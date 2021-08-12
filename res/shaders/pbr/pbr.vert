#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
	vec4 Camera;
} globalUbo;

layout(set = 1, binding = 0) uniform MeshUBO
{
	mat4 Transform;
} meshUbo;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec2 UV;
layout(location = 3) out flat vec3 Camera;

void main()
{
	FragPos = (meshUbo.Transform * vec4(position, 1.0f)).xyz;
	Normal = (transpose(inverse(meshUbo.Transform)) * vec4(normal, 1.0f)).xyz;
	UV = uv;
	Camera = globalUbo.Camera.xyz;

	gl_Position = globalUbo.ToClip * globalUbo.ToCamera * meshUbo.Transform * vec4(position, 1.0f);
}