#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

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
layout(location = 4) out vec3 Tangent;
layout(location = 5) out vec3 Bitangent;

void main()
{
	Camera = globalUbo.Camera.xyz;
	FragPos = vec3(meshUbo.Transform * vec4(position, 1.0f));
	UV = uv;
	Normal = transpose(inverse(mat3(meshUbo.Transform))) * normal;
	Tangent = mat3(meshUbo.Transform) * tangent;
	Bitangent = mat3(meshUbo.Transform) * bitangent;

	gl_Position = globalUbo.ToClip * globalUbo.ToCamera * meshUbo.Transform * vec4(position, 1.0f);
}