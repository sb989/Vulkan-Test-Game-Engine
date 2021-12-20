#version 460

layout(location=0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 currentColor;
layout(location = 1) out vec2 fragTexCoord;
layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 normMatrix;
    mat4 modelView;
    mat4 view;
    mat4 proj;
} ubo;


void main(){
    //UniformBufferObject currentUbo = ubo;//lightBuffer.lights[gl_BaseInstance].ubo;
    //currentColor = lightBuffer.lights[gl_BaseInstance].lightcolor;
    //currentPos = lightBuffer.lights[gl_BaseInstance].lightpos;
    currentColor = inColor;
    fragTexCoord = inTexCoord;
    gl_Position = ubo.proj * ubo.modelView * vec4(inPosition, 1.0);
}