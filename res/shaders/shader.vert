#version 460 core

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 Color;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
} globalUbo;

layout(set = 1, binding = 0) uniform MeshUBO
{
	mat4 Transform;
} meshUbo;

void main()
{
	gl_Position = globalUbo.ToClip * globalUbo.ToCamera * meshUbo.Transform * vec4(position, 1.0f);
}