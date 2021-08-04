#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 Color;
layout(location = 1) in vec3 Camera;

layout(set = 2, binding = 0) uniform MaterialUBO
{
	vec4 Albedo;
} materialUBO;

void main()
{
	outColor = vec4(vec3(Camera.x / 10.0f), 1.0f);
}