#include "vtge_light.hpp"
#include "vtge_model.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_descriptor.hpp"
#include "vtge_graphics.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
static std::vector<Light *> directionalLightList, spotLightList, pointLightList;
VkDescriptorSetLayout *Light::descriptorSetLayout = nullptr;
std::vector<VkBuffer> Light::directionalLightBuffers = {0};
std::vector<VkBuffer> Light::spotLightBuffers = {0};
std::vector<VkBuffer> Light::pointLightBuffers = {0};
std::vector<VkDeviceMemory> Light::directionalLightBuffersMemory = {0};
std::vector<VkDeviceMemory> Light::spotLightBuffersMemory = {0};
std::vector<VkDeviceMemory> Light::pointLightBuffersMemory = {0};
VkDescriptorPool *Light::descriptorPool = nullptr;
std::vector<VkDescriptorSet> *Light::descriptorSets = nullptr;
uint32_t Light::imageCount = 0;

Light::Light(std::string modelPath, glm::vec4 lightPos, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
             float constant, float linear, float quadratic, std::string colorName)
{
    //constructor for a point light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
    //Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    pointLightList.push_back(this);
    m->moveModel(lightPos);
}

Light::Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName)
{
    //constructor for a directional light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->direction = direction;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    directionalLightList.push_back(this);
    m->moveModel(lightPos);
}

Light::Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
             float constant, float linear, float quadratic, float cutOff, float outerCutOff, std::string colorName)
{
    //constructor for a spot light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
    this->cutOff = cutOff;
    this->outerCutOff = outerCutOff;
    //Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    spotLightList.push_back(this);
    m->moveModel(lightPos);
}

void Light::initLights()
{
    setupDescriptorSetLayout();
    Descriptor::createDescriptorBuffer(16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT, &directionalLightBuffers,
                                       &directionalLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT, &spotLightBuffers,
                                       &spotLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT, &pointLightBuffers,
                                       &pointLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    createDescriptorPool();
    createDescriptorSets();
}

Light::~Light()
{
    delete m;
}

void Light::createDirectionalLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 direction, glm::vec4 lightPos,
                                   glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, direction, imgCount, diffuse, ambient, specular, colorName);
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
    //lightList.push_back(m);
}

void Light::createPointLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 lightPos,
                             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant,
                             float linear, float quadratic, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, imgCount, diffuse, ambient,
                         specular, constant, linear, quadratic, colorName);
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
    //lightList.push_back(m);
}

void Light::createSpotLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 direction, glm::vec4 lightPos,
                            glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic,
                            float cutOff, float outerCutOff, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, direction, imgCount, diffuse, ambient,
                         specular, constant, linear, quadratic, cutOff, outerCutOff, colorName);
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
}

void Light::setupDescriptorSetLayout()
{
    auto directionalLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto spotLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto pointLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    // auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
    //     3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {directionalLayoutBinding, spotLayoutBinding, pointLayoutBinding};
    descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Light::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * imageCount},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount}};
    descriptorPool = Descriptor::createDescriptorPool(imageCount, poolSizes);
}

void Light::createDescriptorSets()
{
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo directionalLightBufferInfo{};
    directionalLightBufferInfo.buffer = directionalLightBuffers.data();
    directionalLightBufferInfo.offset = 0;
    directionalLightBufferInfo.range = 16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT;
    directionalLightBufferInfo.binding = 0;
    directionalLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VtgeBufferInfo spotLightBufferInfo{};
    spotLightBufferInfo.buffer = spotLightBuffers.data();
    spotLightBufferInfo.offset = 0;
    spotLightBufferInfo.range = 16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT;
    spotLightBufferInfo.binding = 1;
    spotLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VtgeBufferInfo pointLightBufferInfo{};
    pointLightBufferInfo.buffer = pointLightBuffers.data();
    pointLightBufferInfo.offset = 0;
    pointLightBufferInfo.range = 16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT;
    pointLightBufferInfo.binding = 2;
    pointLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferInfos.push_back(directionalLightBufferInfo);
    bufferInfos.push_back(pointLightBufferInfo);
    bufferInfos.push_back(spotLightBufferInfo);
    Descriptor::populateDescriptorBuffer(descriptorSets, imageCount, bufferInfos, imageInfos);
}

void Light::updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    updateLightBuffers(currentImage, projection, view);
}

// void Light::updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
//     updateLightBuffer(currentImage, projection, view);
// }

void Light::updateLightBuffers(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    updateDirectionalLightBuffer(currentImage, projection, view, directionalLightList, directionalLightBuffersMemory);
    updateSpotLightBuffer(currentImage, projection, view, spotLightList, spotLightBuffersMemory);
    updatePointLightBuffer(currentImage, projection, view, pointLightList, pointLightBuffersMemory);
}

void Light::updateDirectionalLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                         std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(DirectionalLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    DirectionalLightInfo *lightData = (DirectionalLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        lightList[i]->m->updateModelMat(currentImage, projection, view);
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        lightData[i].direction = lightList[i]->direction;
    }
    vkUnmapMemory(device, bufferMemory);
}
void Light::updatePointLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                   std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(PointLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    PointLightInfo *lightData = (PointLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        lightList[i]->m->updateModelMat(currentImage, projection, view);
        lightList[i]->lightPos = lightList[i]->m->getModelPos();
        lightData[i].lightpos = lightList[i]->lightPos;
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        lightData[i].constant = lightList[i]->constant;
        lightData[i].linear = lightList[i]->linear;
        lightData[i].quadratic = lightList[i]->quadratic;
    }
    vkUnmapMemory(device, bufferMemory);
}
void Light::updateSpotLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                  std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(SpotLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    SpotLightInfo *lightData = (SpotLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        lightList[i]->m->updateModelMat(currentImage, projection, view);
        lightList[i]->lightPos = lightList[i]->m->getModelPos();
        lightData[i].lightpos = lightList[i]->lightPos;
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        lightData[i].constant = lightList[i]->constant;
        lightData[i].linear = lightList[i]->linear;
        lightData[i].quadratic = lightList[i]->quadratic;
        lightData[i].direction = lightList[i]->direction;
        lightData[i].cutOff = lightList[i]->cutOff;
        lightData[i].outerCutOff = lightList[i]->outerCutOff;
    }
    vkUnmapMemory(device, bufferMemory);
}

//Light::updateUniformBuffer is no longer needed
void Light::updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    m->updateModelMat(currentImage, projection, view);
}

void Light::drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index, int instance)
{
    std::vector<VkDescriptorSet> combinedDescriptorSets = {};
    m->drawModel(combinedDescriptorSets, commandBuffer, pipelineLayout, index);
}

void Light::drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index)
{
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
}

void Light::recreateAllLights(uint32_t imageCount)
{
    Light::imageCount = imageCount;
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->m->recreateModel(imageCount);
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->m->recreateModel(imageCount);
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->m->recreateModel(imageCount);
    }
    //if(lightList.size() > 0)
    recreateUBufferPoolSets();
}

void Light::recreateUBufferPoolSets()
{
    recreateLightBuffer();
    createDescriptorPool();
    createDescriptorSets();
}

void Light::recreateLightBuffer()
{
    Descriptor::createDescriptorBuffer(16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT, &directionalLightBuffers,
                                       &directionalLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT, &spotLightBuffers,
                                       &spotLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT, &pointLightBuffers,
                                       &pointLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
}

VkDescriptorSetLayout *Light::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

std::vector<VkDescriptorSet> *Light::getDescriptorSets()
{
    return descriptorSets;
}

Light *Light::getDirectionalLight(uint32_t i)
{
    if (directionalLightList.size() > i)
        return directionalLightList[i];
    else
        return nullptr;
}

Light *Light::getPointLight(uint32_t i)
{
    if (pointLightList.size() > i)
        return pointLightList[i];
    else
        return nullptr;
}

std::vector<VkBuffer> *Light::getDirectionalLightBuffers()
{
    return &directionalLightBuffers;
}

std::vector<VkDeviceMemory> *Light::getDirectionalLightBufferMemory()
{
    return &directionalLightBuffersMemory;
}

VkDeviceSize Light::getDirectionalLightBufferSize()
{
    return 16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT;
}

void Light::destroyDescriptorSetLayout()
{
    VkDevice device = Graphics::getDevice();
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete (descriptorSetLayout);
}

void Light::destroyAllLights()
{
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        delete directionalLightList[i];
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        delete spotLightList[i];
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        delete pointLightList[i];
    }

    directionalLightList.clear();
    spotLightList.clear();
    pointLightList.clear();
}

void Light::destroyLightBufferAndMemory(size_t imageCount, std::vector<VkBuffer> buffer, std::vector<VkDeviceMemory> memory)
{
    VkDevice device = Graphics::getDevice();
    for (size_t j = 0; j < imageCount; j++)
    {
        vkDestroyBuffer(device, buffer[j], nullptr);
        vkFreeMemory(device, memory[j], nullptr);
    }
    buffer.clear();
    memory.clear();
}

void Light::cleanupMemory()
{
    VkDevice device = Graphics::getDevice();
    std::cout << "cleaning up model memory" << std::endl;
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->m->cleanupMemory();
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->m->cleanupMemory();
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->m->cleanupMemory();
    }
    vkDestroyDescriptorPool(device, *descriptorPool, nullptr);
    delete descriptorPool;
    delete descriptorSets;
    std::cout << "finised cleaning up model memory" << std::endl;
}

void Light::cleanupAllMemory()
{
    destroyLightBufferAndMemory(imageCount, directionalLightBuffers, directionalLightBuffersMemory);
    destroyLightBufferAndMemory(imageCount, spotLightBuffers, spotLightBuffersMemory);
    destroyLightBufferAndMemory(imageCount, pointLightBuffers, pointLightBuffersMemory);
    cleanupMemory();
}

void Light::setImageCount(uint32_t imageCount)
{
    Light::imageCount = imageCount;
}