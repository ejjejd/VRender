#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 Pos;

layout(set = 3, binding = 0) uniform samplerCube HdrTexture;

void main()
{
    outColor = texture(HdrTexture, normalize(Pos));
}