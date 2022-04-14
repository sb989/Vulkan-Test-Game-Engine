#version 460

layout (location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout (binding = 0) uniform ModelLightMatrix{
    mat4 modelViewLightMat;
    mat4 projLightMat;
}mlm;

// layout(binding = 0) uniform UniformBufferObject{
//     mat4 normMatrix;
//     mat4 modelView;
//     mat4 model;
//     mat4 proj;
// } ubo;

void main()
{
    gl_Position = mlm.projLightMat * mlm.modelViewLightMat * vec4(inPosition, 1.0);
    //gl_Position = ubo.proj * ubo.modelView * vec4(inPosition, 1.0);
}