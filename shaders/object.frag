#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform LightInfo{
    vec3 lightcolor;
    vec3 lightpos;
}lightInfo;

void main() {
    float ambientStrength = 0.1;
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightInfo.lightpos - fragPos);
    vec3 ambient = ambientStrength * lightInfo.lightcolor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightInfo.lightcolor;
    outColor = vec4(ambient+diffuse,1.0) * texture(texSampler, fragTexCoord);
}