#version 460

layout(location=0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 currentColor;
//layout(location = 1) out vec3 currentPos;
struct UniformBufferObject{
    mat4 normMatrix;
    mat4 modelView;
    mat4 view;
    mat4 proj;
};

struct LightInfo{
    vec3 lightcolor;
    vec3 lightpos;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    UniformBufferObject ubo;
};

layout(std140, binding = 0) readonly buffer LightBuffer{
    LightInfo lights[];
} lightBuffer;


void main(){
    UniformBufferObject currentUbo = lightBuffer.lights[gl_BaseInstance].ubo;
    currentColor = lightBuffer.lights[gl_BaseInstance].lightcolor;
    //currentPos = lightBuffer.lights[gl_BaseInstance].lightpos;
    gl_Position = currentUbo.proj * currentUbo.modelView * vec4(inPosition, 1.0);
}