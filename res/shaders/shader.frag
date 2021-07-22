#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0)in vec3 Color;

void main()
{
	outColor = vec4(Color, 1.0f);
}