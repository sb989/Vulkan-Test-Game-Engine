#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragNormal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec4 shadowFragPos;

//layout(location = 4) out vec3 camPos;

layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 normMatrix;
    mat4 modelView;
    mat4 model;
    mat4 proj;
} ubo;



void main() {
    gl_Position = ubo.proj * ubo.modelView * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = ubo.normMatrix * vec4(inNormal,0);//mat3(transpose(inverse(ubo.model))) * inNormal;
    fragPos = vec3(ubo.modelView * vec4(inPosition, 1.0));
    shadowFragPos = vec4(inPosition, 1.0);
}