#version 460 core

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ToCamera;
	mat4 ToClip;
	vec4 Camera;
} globalUbo;

layout(set = 1, binding = 0) uniform MeshUBO
{
	mat4 Transform;
} meshUbo;

layout(location = 0) out vec3 Pos;

void main()
{
    Pos = position;

    mat4 cameraTransform = mat4(mat3(globalUbo.ToCamera)); 
    mat4 meshTransform = mat4(mat3(meshUbo.Transform));

    gl_Position = (globalUbo.ToClip * cameraTransform * meshTransform * vec4(position, 1.0f)).xyww;
}