#version 450

layout(location=0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(binding=1) uniform UniformBufferObject{
    mat4 normMatrix;
    mat4 modelView;
    mat4 view;
    mat4 proj;
} ubo;


void main(){
    gl_Position = ubo.proj * ubo.modelView * vec4(inPosition, 1.0);
}