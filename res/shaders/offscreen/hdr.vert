#version 460 core

vec2 vertices[] = vec2[]
(
    vec2(-1.0f, 1.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(-1.0f, -1.0f),
    vec2(-1.0f, 1.0f)
);

layout(location = 0) out vec2 UV;

void main()
{
    vec2 pos = vertices[gl_VertexIndex];

    UV = pos * 0.5f + 0.5f;

    gl_Position = vec4(pos, 0.0f, 1.0f);
}