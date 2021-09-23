#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_swapchain.hpp"
#include <iostream>
static std::vector <Object*>objectList{};

Object::Object(std::string modelPath, std::string texturePath, Swapchain * swapchain){
    this->m = new Model(modelPath, texturePath, swapchain);
    objectList.push_back(this);
    isVisible = true;
}   

Object::Object(std::string modelPath, Swapchain * swapchain){
    this->m = new Model(modelPath, swapchain);
    objectList.push_back(this);
    isVisible = true;
}   

Object::~Object(){
    delete m;
}

void Object::destroyAllObjects(){
    for(int i = 0; i < objectList.size(); i++){
        delete objectList[i];
    }
    objectList.clear();
}

void Object::drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int count){
    VkBuffer vertexBuffers[] = {m->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &(*m->descriptorSets)[count], 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(m->vertexIndices.size()), 1, 0, 0, 0);
}

void Object::drawAllObjects(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int count){
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < objectList.size(); i++){
        if(objectList[i]->getIsVisible())
            objectList[i]->drawObject(commandBuffer, pipeline->getPipelineLayout(), count);
    }
}

void Object::cleanupAllModelMemory(){
    for (int i = 0; i < objectList.size(); i++){
        objectList[i]->m->cleanupMemory();
    }
}

void Object::updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    m->updateUniformBuffer(currentImage, projection, view);
}

void Object::updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->updateObject(currentImage, projection, view);
    }
}

void Object::recreateModel(Swapchain *swapchain){
    m->recreateUBufferPoolSets(swapchain);
}

void Object::recreateAllObjectsModel(Swapchain *swapchain){
    std::cout<<objectList.size()<<std::endl;
    if(objectList.size() > 0){
        objectList[0]->m->setSwapchain(swapchain);
        objectList[0]->m->recreateLightBuffer();
    }
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->m->recreateUBufferPoolSets(swapchain);
    }
}