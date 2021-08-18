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

layout(location = 0) out vec3 UV;

void main()
{
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
}