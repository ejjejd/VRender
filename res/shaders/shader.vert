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
	Color = vec3(0.2f);
	
	gl_Position = ubo.ToClip * ubo.ToCamera * vec4(vec3(position.x, position.y, position.z), 1.0f);
}