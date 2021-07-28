#version 460 core

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 Color;

layout(binding = 0) uniform UboObject
{
	mat4 ToCamera;
	mat4 ToClip;
} ubo;

void main()
{
	Color = position * 0.5f + 0.5f;
	
	gl_Position = ubo.ToClip * ubo.ToCamera * vec4(position, 1.0f);
}