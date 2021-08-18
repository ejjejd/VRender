#version 460 core

layout(location = 0) out vec4 OutputColor;

layout(binding = 0) uniform sampler2D Texture;

void main()
{
    OutputColor = vec4(1.0f, 0.2f, 0.2f, 1.0f);
}