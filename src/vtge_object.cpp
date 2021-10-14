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
VkDescriptorSetLayout * Object::descriptorSetLayout = nullptr;
Material defaultMaterial = {
    glm::vec3(0.5,0.5,0.5),
    glm::vec3(0.5,0.5,0.5),
    glm::vec3(0.7,0.7,0.7),
    32.0f
};
Material blankMaterial = {
    glm::vec3(0,0,0),
    glm::vec3(0,0,0),
    glm::vec3(0.5,0.5,0.5),
    32.0f
};
Object::Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath, std::string specularMapPath, glm::vec4 color){
    
    this->m = new Model(modelPath, diffuseMapPath, specularMapPath, color);
    this->imageCount = imageCount;
    objectList.push_back(this);
    isVisible = true;
    if(!descriptorSetLayout){
        setupDescriptorSetLayout();
    }
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
    Material mtrl;
    if(diffuseMapPath != "" || color != glm::vec4(0,0,0,255))
        mtrl = blankMaterial;
    else
        mtrl = defaultMaterial;
    if(specularMapPath != "")
        mtrl.specular = glm::vec3(0);
    updateMaterial(mtrl);
}

Object::~Object(){
    delete m;
}

void Object::setupDescriptorSetLayout(){
    auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    auto samplerLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto lightLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto materialLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto specMapLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {lightLayoutBinding, uboLayoutBinding, samplerLayoutBinding, materialLayoutBinding, specMapLayoutBinding};
    Object::descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Object::createDescriptorBuffers(){
    Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(sizeof(Material), &material, &materialMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
}

void Object::createDescriptorPool(){
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, imageCount},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * imageCount},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * imageCount},
    };
    descriptorPool = Descriptor::createDescriptorPool(imageCount, poolSizes);
}

void Object::createDescriptorSets(){
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo lightBuffer {};
    lightBuffer.buffer = Light::getLightBuffers()->data();
    lightBuffer.offset = 0;
    lightBuffer.range = Light::getLightBufferSize();
    lightBuffer.binding = 0;
    lightBuffer.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VtgeBufferInfo uniformBuffer {};
    uniformBuffer.buffer = uniformBuffers.data();
    uniformBuffer.offset = 0;
    uniformBuffer.range = sizeof(UniformBufferObject);
    uniformBuffer.binding = 1;
    uniformBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VtgeBufferInfo materialBuffer {};
    materialBuffer.buffer = material.data();
    materialBuffer.offset = 0;
    materialBuffer.range = sizeof(Material);
    materialBuffer.binding = 3;
    materialBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferInfos.push_back(lightBuffer);
    bufferInfos.push_back(uniformBuffer);
    bufferInfos.push_back(materialBuffer);
    VtgeImageInfo imageBuffer{};
    imageBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBuffer.view = m->diffuseMap->textureImageView;
    imageBuffer.sampler = m->diffuseMap->textureSampler;
    imageBuffer.binding = 2;
    imageBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VtgeImageInfo specMapBuffer{};
    specMapBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    specMapBuffer.view = m->specularMap->textureImageView;
    specMapBuffer.sampler = m->specularMap->textureSampler;
    specMapBuffer.binding = 4;
    specMapBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageInfos.push_back(imageBuffer);
    imageInfos.push_back(specMapBuffer);
    
    Descriptor::populateDescriptorBuffer(descriptorSets,imageCount, bufferInfos, imageInfos);
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
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &(*descriptorSets)[index], 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(m->vertexIndices.size()), 1, 0, 0, 0);
}

void Object::updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->updateObject(currentImage, projection, view);
    }
}

void Object::updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    updateUniformBuffer(currentImage, projection, view);
}

void Object::updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    //using push constants is a more efficent way to pass a small buffer of data to shaders
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currrentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currrentTime - startTime).count();
    UniformBufferObject ubo{};
    m->updateModelMat();
    ubo.modelView = view * m->getModelMat();
    ubo.proj = projection;
    ubo.view = view;
    ubo.normMatrix = transpose(inverse(ubo.modelView));
    //ubo.proj[1][1] *= -1; //glm was designed for opengl where y coords are inverted so multiply by -1
    void *data;
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

void Object::updateMaterial(Material mat){
    for(int currentImage = 0; currentImage < imageCount; currentImage++){
        void * data;
        vkMapMemory(device, materialMemory[currentImage], 0, sizeof(mat), 0, &data);
        memcpy(data, &mat, sizeof(mat));
        vkUnmapMemory(device, materialMemory[currentImage]);
    }
}

void Object::setImageCount(uint32_t imageCount){
    this->imageCount = imageCount;
}

VkDescriptorSetLayout * Object::getDescriptorSetLayout(){
    return descriptorSetLayout;
}

void Object::recreateAllObjects(uint32_t imageCount){
    std::cout<<objectList.size()<<std::endl;
    for(int i = 0; i < objectList.size(); i++){
        objectList[i]->recreateUBufferPoolSets(imageCount);
    }
}

void Object::recreateUBufferPoolSets(uint32_t imageCount){
    this->imageCount = imageCount;
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Object::destroyDescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete(descriptorSetLayout);
}

void Object::cleanupMemory(){
    std::cout<<"cleaning up model memory"<<std::endl;
    for(size_t j = 0; j < imageCount; j++){
        vkDestroyBuffer(device, uniformBuffers[j], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[j], nullptr);
        vkDestroyBuffer(device, material[j], nullptr);
        vkFreeMemory(device, materialMemory[j], nullptr);
    }
    vkDestroyDescriptorPool(device,*descriptorPool,nullptr);
    delete descriptorPool;
    delete descriptorSets;
    std::cout<<"finised cleaning up model memory"<<std::endl;
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


// void Object::createMaterial(){
//     for(size_t i = 0; i < m->swapchain->swapchainImages.size(); i++){
//         VkDescriptorBufferInfo materialInfo{};
//         materialInfo.buffer = material[i];
//         materialInfo.offset = 0;
//         materialInfo.range = sizeof(Materials);
//         std::vector<VkWriteDescriptorSet> descriptorWrites{};
//         descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
//             VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*m->descriptorSets)[i], 3,
//             0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &materialInfo, nullptr, nullptr));
//         Descriptor::updateDescriptorSets(descriptorWrites);
//     }
// }