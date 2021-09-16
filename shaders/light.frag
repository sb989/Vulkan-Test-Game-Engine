#version 450

layout(location = 0) out vec4 outColor;
layout(binding = 2) uniform LightInfo{
    vec3 lightcolor;
    vec3 lightpos;
}lightInfo;

void main(){
    outColor = vec4(lightInfo.lightcolor,1);
}
