#include "vtge_light.hpp"
#include "vtge_model.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_descriptor.hpp"
#include <iostream>
extern VkDevice device;
//extern VkDescriptorSetLayout descriptorSetLayout;
static std::vector<Light*> lightList {};
Light::Light(std::string modelPath, Swapchain *swapchain, glm::vec3 lightColor, glm::vec3 lightPos){
    this->m = new Model(modelPath, swapchain);
    this->lightColor = lightColor;
    this->lightPos = lightPos;
    // if(lightBuffers.empty()){
    //     //createStorageBuffers(swapchain);
    //     setupDescriptorSetLayout();
    // }
    // createDescriptorPool(swapchain);
    // createDescriptorSets(swapchain);
    lightList.push_back(this);
    m->moveModel(lightPos);
}

Light::~Light(){
    delete m;
}

void Light::destroyAllLights(){
    for(int i = 0; i < lightList.size(); i++){
        delete lightList[i];
    }
    lightList.clear();
}

// void Light::createDescriptorPool(Swapchain *swapchain){
//     std::vector<VkDescriptorPoolSize> poolSizes = {
//         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(swapchain->swapchainImages.size())}
//     };
//     descriptorPool = Descriptor::createDescriptorPool(swapchain, poolSizes);
// }

// void Light::setupDescriptorSetLayout(){
//      auto lightLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
//         0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
//     std::vector<VkDescriptorSetLayoutBinding> bindings = {lightLayoutBinding};
//     descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
// }

// void Light::createDescriptorSets(Swapchain *swapchain){
//     descriptorSets = Descriptor::allocateDescriptorSets(swapchain, *descriptorSetLayout, *descriptorPool);
//     for(size_t i = 0; i < swapchain->swapchainImages.size(); i++){
//         VkDescriptorBufferInfo lightInfo{};
//         lightInfo.buffer = lightBuffers[i];
//         lightInfo.offset = 0;//? might have to change not sure how storage buffers work
//         lightInfo.range = sizeof(LightInfo) * MAX_LIGHT_COUNT;
//         std::vector<VkWriteDescriptorSet> descriptorWrites{};
//         descriptorWrites[0] = Descriptor::createWriteDescriptorSet(
//             VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 0,
//             0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &lightInfo, nullptr, nullptr);
//         //change the second one to be a light buffer
//         Descriptor::updateDescriptorSets(descriptorWrites);           
//     } 
// }



// void Light::recreateUBufferPoolSets(Swapchain *swapchain){
//     createDescriptorPool(swapchain);
//     createDescriptorSets(swapchain);
// }

void Light::recreateAllLights(Swapchain *swapchain){
    for(int i = 0; i < lightList.size(); i++){
        //lightList[i]->recreateUBufferPoolSets(swapchain);
        lightList[i]->m->recreateUBufferPoolSets(swapchain);
    }
}

void Light::updateUniformBuffer(uint32_t currentImage){
    void *data;
    // LightInfo li{};
    // li.lightcolor = glm::vec3(1.0f, 1.0f, 1.0f);
    // li.lightpos = lightPos;
    VkDeviceMemory bufferMemory = (*Model::getLightBufferMemory())[currentImage];
    vkMapMemory(device, bufferMemory, 0, sizeof(LightInfo) * lightList.size(), 0, &data);
    LightInfo *lightData = (LightInfo*) data;
    for(int i = 0; i < lightList.size(); i++){
        lightData[i].lightpos = lightList[i]->lightPos;
        lightData[i].lightcolor = lightList[i]->lightColor;
    }
    vkUnmapMemory(device, bufferMemory);
}

void Light::updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    updateUniformBuffer(currentImage);
    m->updateUniformBuffer(currentImage, projection, view);
}

void Light::updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    for(int i = 0; i < lightList.size(); i++){
        lightList[i]->updateLight(currentImage, projection, view);
    }
}

void Light::drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int count){
    VkBuffer vertexBuffers[] = {m->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &(*(m->descriptorSets))[count], 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(m->vertexIndices.size()), 1, 0, 0, 0);
}

void Light::drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int count){
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < lightList.size(); i++){
        lightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), count);
    }
}

void Light::cleanupMemory(Swapchain *swapchain){
    for(size_t j = 0; j<swapchain->swapchainImages.size(); j++){
        vkDestroyBuffer(device, m->uniformBuffers[j], nullptr);
        vkFreeMemory(device, m->uniformBuffersMemory[j], nullptr);
    }
    vkDestroyDescriptorPool(device,*m->descriptorPool,nullptr);
}

void Light::cleanupAllMemory(Swapchain *swapchain){
    int size = lightList.size();
    for (int i = 0; i < size; i++){
        lightList[i]->m->cleanupMemory();
    }
    Model::destroyLightBufferAndMemory(swapchain->swapchainImages.size());
    // for(size_t j = 0; j<swapchain->swapchainImages.size(); j++){
    //     vkDestroyBuffer(device, (*Model::getLightBuffers())[j], nullptr);
    //     vkFreeMemory(device, (*Model::getLightBufferMemory())[j],nullptr);
    // }
}