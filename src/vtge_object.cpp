#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_texture.hpp"
#include <iostream>
#include <vtge_descriptor.hpp>
#include <vtge_light.hpp>
#include <cstring>
static std::vector <Object*>objectList{};
extern VkDevice device;
Object::Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath, std::string specularMapPath, glm::vec4 color){
    
    this->m = new Model(modelPath, imageCount, diffuseMapPath, specularMapPath, color);
    this->imageCount = imageCount;
    objectList.push_back(this);
    isVisible = true;
    
}

Object::~Object(){
    delete m;
}

void Object::drawAllObjects(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index){
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < objectList.size(); i++){
        if(objectList[i]->getIsVisible())
            objectList[i]->drawObject(commandBuffer, pipeline->getPipelineLayout(), index);
    }
}

void Object::drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index){
    VkBuffer vertexBuffers[] = {m->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    VkDescriptorSet combindedDescriptorSets [2] = {(*m->getDescriptorSets())[index], (*Light::getDescriptorSets())[index]};
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 2, combindedDescriptorSets, 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(m->vertexIndices.size()), 1, 0, 0, 0);
}

void Object::updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->updateObject(currentImage, projection, view);
    }
}

void Object::updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    //updateUniformBuffer(currentImage, projection, view);
    m->updateModelMat(currentImage, projection, view);
}

void Object::recreateAllObjects(uint32_t imageCount){
    std::cout<<objectList.size()<<std::endl;
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->m->recreateUBufferPoolSets(imageCount);
    }
}

void Object::cleanupMemory(){
    m->cleanupMemory();
}

void Object::destroyAllObjects(){
    for(int i = 0; i < objectList.size(); i++){
        delete objectList[i];
    }
    objectList.clear();
}

void Object::cleanupAllMemory(){
    for (int i = 0; i < objectList.size(); i++){
        objectList[i]->cleanupMemory();
    }
}
