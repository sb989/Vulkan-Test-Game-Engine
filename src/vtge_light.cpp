#include "vtge_light.hpp"
#include "vtge_model.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_descriptor.hpp"
#include <iostream>
extern VkDevice device;
//extern VkDescriptorSetLayout descriptorSetLayout;
static std::vector<Light*> lightList {};
VkDescriptorSetLayout * Light::descriptorSetLayout = nullptr;
std::vector<VkBuffer> Light::lightBuffers = {0};
std::vector<VkDeviceMemory> Light::lightBuffersMemory = {0};
VkDescriptorPool * Light::descriptorPool = nullptr;
std::vector<VkDescriptorSet> * Light::descriptorSets = nullptr;
Light::Light(std::string modelPath, glm::vec3 lightColor, glm::vec3 lightPos, uint32_t imageCount,
    glm::vec3 diffuse, glm::vec3 ambient, glm::vec3 specular){
    this->m = new Model(modelPath);
    this->lightColor = lightColor;
    this->lightPos = lightPos;
    this->imageCount = imageCount;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    if(!descriptorSetLayout){
        setupDescriptorSetLayout();
        Descriptor::createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers,
            &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
        createDescriptorPool();
        createDescriptorSets();
    }
    lightList.push_back(this);
    m->moveModel(lightPos);
}

Light::~Light(){
    delete m;
}

void Light::setupDescriptorSetLayout(){
    auto lightLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    // auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
    //     1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {lightLayoutBinding};
    descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Light::createDescriptorPool(){
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, imageCount}
    };
    descriptorPool = Descriptor::createDescriptorPool(imageCount, poolSizes);
}

void Light::createDescriptorSets(){
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo lightBuffer {};
    lightBuffer.buffer = lightBuffers.data();
    lightBuffer.offset = 0;
    lightBuffer.range = sizeof(LightInfo) * MAX_LIGHT_COUNT;
    lightBuffer.binding = 0;
    lightBuffer.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferInfos.push_back(lightBuffer);
    Descriptor::populateDescriptorBuffer(descriptorSets,imageCount, bufferInfos, imageInfos);
}

void Light::updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    updateLightBuffer(currentImage, projection, view);
}

// void Light::updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
//     updateLightBuffer(currentImage, projection, view);
// }

void Light::updateLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    void *data;
   
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, sizeof(LightInfo) * lightList.size(), 0, &data);
    LightInfo *lightData = (LightInfo*) data;
    for(int i = 0; i < lightList.size(); i++){
        lightList[i]->updateUniformBuffer(currentImage, projection, view);
        lightData[i].lightpos = lightList[i]->lightPos;
        lightData[i].lightcolor = lightList[i]->lightColor;
        lightData[i].ubo = lightList[i]->ubo;
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        
    }
    vkUnmapMemory(device, bufferMemory);
}

void Light::updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    //using push constants is a more efficent way to pass a small buffer of data to shaders
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currrentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currrentTime - startTime).count();
    //UniformBufferObject ubo{};
    m->updateModelMat();
    lightPos = m->getModelPos();
    ubo.modelView = view * m->getModelMat();
    ubo.proj = projection;
    ubo.view = view;
    ubo.normMatrix = transpose(inverse(ubo.modelView));
    //ubo.proj[1][1] *= -1; //glm was designed for opengl where y coords are inverted so multiply by -1
    // void *data;
    // vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    // memcpy(data, &ubo, sizeof(ubo));
    // vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

void Light::drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index, int instance){
    VkBuffer vertexBuffers[] = {m->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &(*(descriptorSets))[index], 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(m->vertexIndices.size()), 1, 0, 0, instance);
}

void Light::drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index){
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < lightList.size(); i++){
        lightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
}


void Light::recreateAllLights(uint32_t imageCount){
    for(int i = 0; i < lightList.size(); i++){
        lightList[i]->imageCount = imageCount;
    }
    if(lightList.size() > 0)
        lightList[0]->recreateUBufferPoolSets();
}

void Light::recreateUBufferPoolSets(){
    recreateLightBuffer();
    createDescriptorPool();
    createDescriptorSets();
}

void Light::recreateLightBuffer(){
    Descriptor::createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
}

VkDescriptorSetLayout * Light::getDescriptorSetLayout(){
    return descriptorSetLayout;
}

Light * Light::getLight(uint32_t i){
    return lightList[i];
}

std::vector<VkBuffer> * Light::getLightBuffers(){
    return &lightBuffers;
}

std::vector<VkDeviceMemory> * Light::getLightBufferMemory(){
    return &lightBuffersMemory;
}

VkDeviceSize Light::getLightBufferSize(){
    return sizeof(LightInfo) * MAX_LIGHT_COUNT;
}

void Light::destroyDescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete(descriptorSetLayout);
}

void Light::destroyAllLights(){
    for(int i = 0; i < lightList.size(); i++){
        delete lightList[i];
    }
    lightList.clear();
}

void Light::destroyLightBufferAndMemory(size_t imageCount){
    for(size_t j = 0; j < imageCount; j++){
        vkDestroyBuffer(device, lightBuffers[j], nullptr);
        vkFreeMemory(device, lightBuffersMemory[j], nullptr);
    }
    lightBuffers.clear();
    lightBuffersMemory.clear();
}

void Light::cleanupMemory(){
    std::cout<<"cleaning up model memory"<<std::endl;
    vkDestroyDescriptorPool(device,*descriptorPool,nullptr);
    delete descriptorPool;
    delete descriptorSets;
    std::cout<<"finised cleaning up model memory"<<std::endl;
}



void Light::cleanupAllMemory(){
    if(lightList[0])
        destroyLightBufferAndMemory(lightList[0]->imageCount);
    cleanupMemory();
    // for(size_t j = 0; j<swapchain->swapchainImages.size(); j++){
    //     vkDestroyBuffer(device, (*Model::getLightBuffers())[j], nullptr);
    //     vkFreeMemory(device, (*Model::getLightBufferMemory())[j],nullptr);
    // }
}