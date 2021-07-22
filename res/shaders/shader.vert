#version 460 core

vec2 positions[] = 
{
	vec2(-0.5f, 0.5f),
	vec2(0.0f, -0.5f),
	vec2(0.5f, 0.5f)
};

layout(location = 0)out vec3 Color;

void main()
{
	vec2 pos = positions[gl_VertexIndex];

	Color = vec3(pos * 0.5f + 0.5f, 0.0f);
	
	gl_Position = vec4(pos, 0.0f, 1.0f);
}