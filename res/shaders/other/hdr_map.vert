#version 460 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
	vec4 Camera;
} globalUbo;

layout(location = 0) out vec3 Pos;

void main()
{
    Pos = position;

    mat4 cameraTransform = mat4(mat3(globalUbo.ToCamera)); 
    gl_Position = (globalUbo.ToClip * cameraTransform * vec4(position, 1.0f)).xyww;
}