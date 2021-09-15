#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(binding = 1) uniform sampler2D texSampler;
vec3 lightcolor = vec3(1.0);

void main() {
    outColor = vec4(lightcolor,1.0) * texture(texSampler, fragTexCoord);
}