#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 Normal;
layout(location = 1) in vec3 Camera;

layout(set = 2, binding = 0) uniform MaterialUBO
{
	vec3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;
} materialUBO;

void main()
{
	outColor = vec4(materialUBO.Albedo, 1.0f);
}