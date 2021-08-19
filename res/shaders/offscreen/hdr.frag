#version 460 core

layout(location = 0) out vec4 OutputColor;

layout(location = 0) in vec2 UV;

layout(binding = 0) uniform sampler2D Texture;

void main()
{
    vec4 color = texture(Texture, UV);
    OutputColor = vec4(color.xyz, 1.0f);
}