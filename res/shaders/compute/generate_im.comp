#version 460 core

layout(local_size_x = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

void main()
{
    vec4 pixel = vec4(float(gl_GlobalInvocationID.x) / float(gl_NumWorkGroups.x), float(gl_GlobalInvocationID.y) / float(gl_NumWorkGroups.y), 0.0, 1.0);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    imageStore(img, pixelCoord, pixel);
}