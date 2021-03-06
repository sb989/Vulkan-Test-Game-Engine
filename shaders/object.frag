#version 460
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragNormal;
layout(location = 3) in vec3 fragPos;

struct SpotLightInfo{
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    vec4 direction;
    vec4 lightpos;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

struct PointLightInfo{
    float constant;
    float linear;
    float quadratic;
    vec4 lightpos;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

struct DirectionalLightInfo{
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
    vec4 direction;
};

layout(std140, set = 1, binding = 0) readonly buffer DirectionalLightBuffer{
    int numLights;
    DirectionalLightInfo lights[];
} directionalLightBuffer;

layout(std140, set = 1, binding = 1) readonly buffer SpotLightBuffer{
    int numLights;
    SpotLightInfo lights[];
} spotLightBuffer;

layout(std140, set = 1, binding = 2) readonly buffer PointLightBuffer{
    int numLights;
    PointLightInfo lights[];
} pointLightBuffer;


layout(set = 0, binding = 1) uniform sampler2D texSampler;// diffuse map/texture
layout(set = 0, binding = 2) uniform sampler2D specSampler;

layout(set = 0, binding =3) uniform Material{
    float shininess;
}material;

vec3 calcDirLight(DirectionalLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec);
vec3 calcPointLight(PointLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec);
vec3 calcSpotLight(SpotLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec);
void main() {
    vec4 objColor = texture(texSampler, fragTexCoord);
    vec4 objSpec = texture(specSampler, fragTexCoord);
    vec3 norm = normalize(vec3(fragNormal));
    vec3 colorSum = vec3(0);
    for(int i = 0; i < directionalLightBuffer.numLights; i++){
        DirectionalLightInfo dirLight = directionalLightBuffer.lights[i];
        colorSum += calcDirLight(dirLight, norm, objColor, objSpec);
    }
    for(int i = 0; i < pointLightBuffer.numLights; i++){
        PointLightInfo pointLight = pointLightBuffer.lights[i];
        colorSum += calcPointLight(pointLight, norm, objColor, objSpec);
    }
    for(int i = 0; i < spotLightBuffer.numLights; i++){
        SpotLightInfo spotLight = spotLightBuffer.lights[i];
        colorSum += calcSpotLight(spotLight, norm, objColor, objSpec);
    }
    outColor = vec4(colorSum,1.0);
}

vec3 calcDirLight(DirectionalLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec){
    vec3 dirToLight = normalize((vec3(-light.direction)));
    //viewDir = viewPos - fragPos, but camera is always at the origin 
    // in view space, so it becomes viewDir = -fragPos
    vec3 viewDir = normalize(-fragPos); 
    float diff = max(dot(normal, dirToLight), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-dirToLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 amb = vec3(light.ambient);
    vec3 ambient  = amb * vec3(objColor);
    vec3 diffuse  = vec3(light.diffuse  * (diff * objColor));
    vec3 specular = vec3(light.specular * (spec * objSpec));
    
    vec3 result = ambient + diffuse + specular;
    return result;
}

vec3 calcPointLight(PointLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec){
    vec3 viewLightPos = vec3(light.lightpos);
    vec3 lightPosFragPosDiff =  viewLightPos - fragPos;
    vec3 lightDir = normalize(lightPosFragPosDiff);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float dist = length(lightPosFragPosDiff);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    vec3 ambient = vec3(light.ambient * objColor);
    vec3 diffuse = vec3(light.diffuse * (diff * objColor));
    vec3 specular = vec3(light.specular * (spec * objSpec));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    vec3 result = diffuse + specular + ambient;
    return result;
}

vec3 calcSpotLight(SpotLightInfo light, vec3 normal, vec4 objColor, vec4 objSpec)
{
    vec3 viewLightPos = vec3(light.lightpos);
    vec3 lightPosFragPosDiff =  viewLightPos - fragPos;
    vec3 lightDir = normalize(lightPosFragPosDiff);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float dist = length(lightPosFragPosDiff);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(vec3(-light.direction))); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // // combine results
    vec3 ambient = vec3(light.ambient * objColor);
    vec3 diffuse = vec3(light.diffuse * (diff * objColor));
    vec3 specular = vec3(light.specular * (spec * objSpec));
     ambient *=  intensity;
     diffuse *= intensity;
     specular *=  intensity;
    vec3 result = (diffuse + ambient + specular);
    return result;
    }