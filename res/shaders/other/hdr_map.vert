#version 460 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
	vec4 Camera;
} globalUbo;

layout(location = 0) out vec2 UV;

void main()
{
    UV = uv;

    mat4 transform = mat4(mat3(globalUbo.ToClip)); 
    gl_Position = globalUbo.ToCamera * globalUbo.ToClip * vec4(position, 1.0f);
}