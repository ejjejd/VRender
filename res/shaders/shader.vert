#version 460 core

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 Color;

void main()
{
	Color = position * 0.5f + 0.5f;
	
	gl_Position = vec4(position, 1.0f);
}