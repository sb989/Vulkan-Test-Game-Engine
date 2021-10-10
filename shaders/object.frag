#version 460

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in mat4 view;

struct UniformBufferObject{
    mat4 normMatrix;
    mat4 modelView;
    mat4 view;
    mat4 proj;
};

struct LightInfo{
    vec3 lightcolor;
    vec3 lightpos;
    UniformBufferObject ubo;
};

layout(std140, binding = 0) readonly buffer LightBuffer{
    LightInfo lights[];
} lightBuffer;

layout(binding = 2) uniform sampler2D texSampler;
// layout(binding = 2) uniform LightInfo{
//     vec3 lightcolor;
//     vec3 lightpos;
// }lightInfo;
void main() {
    //vec3 lpos = vec3(0,0,0);
    //vec3 lcolor = vec3(1,1,1);
    vec3 lpos = lightBuffer.lights[0].lightpos;
    vec3 lcolor = lightBuffer.lights[0].lightcolor;
    vec3 viewLightPos = vec3(view * vec4(lpos,1.0));
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(viewLightPos - fragPos);
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lcolor;
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lcolor;

    float specularStrength = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lcolor;

    outColor = vec4(ambient+diffuse+specular,1.0) * vec4(0.5,0.5,0.5,1);//texture(texSampler, fragTexCoord);
    //vec4(0.5,0.5,0.5,1) the color grey
}