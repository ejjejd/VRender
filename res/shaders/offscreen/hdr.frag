#version 460 core

layout(location = 0) out vec4 OutputColor;

layout(location = 0) in vec2 UV;

layout(binding = 0) uniform sampler2D Texture;

void main()
{
    vec3 color = texture(Texture, UV).xyz;

    //Tone mapping
    const float exposure = 0.6f;
    color = vec3(1.0f) - exp(-color * exposure);

    OutputColor = vec4(color, 1.0f);
}