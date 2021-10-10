#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 currentColor;
// layout(binding = 2) uniform LightInfo{
//     vec3 lightcolor;
//     vec3 lightpos;
// }lightInfo;

void main(){
    outColor = vec4(currentColor, 1);
}
