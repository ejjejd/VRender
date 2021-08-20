#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 UV;

void main()
{
    outColor = vec4(vec2(1.0f), 0.0f, 1.0f);
}