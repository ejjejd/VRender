#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 Pos;

layout(set = 3, binding = 0) uniform sampler2D HdrTexture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 FetchFromSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5f;
    return uv;
}

void main()
{
    vec2 uv = FetchFromSphericalMap(normalize(Pos));
    outColor = texture(HdrTexture, uv);
}