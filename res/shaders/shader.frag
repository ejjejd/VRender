#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0)in vec3 Color;

layout(set = 2, binding = 0) uniform MaterialUBO
{
	vec4 Albedo;
} materialUBO;

void main()
{
	outColor = vec4(materialUBO.Albedo.xyz, 1.0f);
}