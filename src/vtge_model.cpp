#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include <vtge_model.hpp>
#include <vtge_texture.hpp>
#include <vtge_swapchain.hpp>
#include <array>
#include <cstring>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "vtge_descriptor.hpp"
extern VkDevice device;
//extern VkDescriptorSetLayout descriptorSetLayout;
Model::Model(std::string modelPath, std::string texturePath, Swapchain *swapchain){
    this->modelPath = modelPath;
    this->texturePath = texturePath;
    this->swapchain = swapchain;
    this->texture = new Texture(texturePath);
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
    if(!descriptorSetLayout){
        setupDescriptorSetLayout();
        createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

Model::Model(std::string modelPath, Swapchain *swapchain){
    this->modelPath = modelPath;
    this->swapchain = swapchain;
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
    this->texture = NULL;
    if(!descriptorSetLayout){
        setupDescriptorSetLayout();
        createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

Model::~Model(){
    if(texture)
        delete texture;
    //vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void Model::recreateUBufferPoolSets(Swapchain *swapchain){
    this->swapchain = swapchain;
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Model::createDescriptorBuffer(VkDeviceSize bufferSize, std::vector<VkBuffer> *buffers, std::vector<VkDeviceMemory> *bufferMemory, VkBufferUsageFlags bufferUsage){
    buffers->resize(swapchain->swapchainImages.size());
    bufferMemory->resize(swapchain->swapchainImages.size());
    for(size_t i =0; i<swapchain->swapchainImages.size(); i++){
        buffer::createBuffer(bufferSize, bufferUsage, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        (*buffers)[i], (*bufferMemory)[i]);
    } 
}

void Model::createUniformBuffers(){
    createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

void Model::createDescriptorSets(){
    descriptorSets = Descriptor::allocateDescriptorSets(swapchain, *descriptorSetLayout, *descriptorPool);
    for(size_t i = 0; i < swapchain->swapchainImages.size(); i++){
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        VkDescriptorBufferInfo lightInfo{};
        lightInfo.buffer = lightBuffers[i];
        lightInfo.offset = 0;//? might have to change not sure how storage buffers work
        lightInfo.range = sizeof(LightInfo) * MAX_LIGHT_COUNT;
        if(texture){
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->textureImageView;
            imageInfo.sampler = texture->textureSampler;
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 0,
                0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &lightInfo, nullptr, nullptr));
            descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 1,
                0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo, nullptr, nullptr));
            descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 2,
                0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, nullptr, &imageInfo, nullptr));
            Descriptor::updateDescriptorSets(descriptorWrites);
        } else{
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 0,
                0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &lightInfo, nullptr, nullptr));
            descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, (*descriptorSets)[i], 1,
                0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo, nullptr, nullptr));
            Descriptor::updateDescriptorSets(descriptorWrites);
        }        
    }
}

void Model::createDescriptorPool(){
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(swapchain->swapchainImages.size())},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapchain->swapchainImages.size())},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapchain->swapchainImages.size())}
    };
    descriptorPool = Descriptor::createDescriptorPool(swapchain, poolSizes);
}

void Model::createVertexBuffer(){
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBufferAndCopy(bufferSize, &vertexBuffer, &vertexBufferMemory, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.data());
}

void Model::createIndexBuffer(){
    VkDeviceSize bufferSize = sizeof(vertexIndices[0]) * vertexIndices.size();
    createBufferAndCopy(bufferSize, &indexBuffer, &indexBufferMemory, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vertexIndices.data());
}

void Model::createBufferAndCopy(VkDeviceSize bufferSize, VkBuffer *buffer, VkDeviceMemory *deviceMemory, VkBufferUsageFlags flags,void *pointer){
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    buffer::createStagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pointer, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    buffer::createBuffer(bufferSize, flags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *deviceMemory);
    buffer::copyBuffer(stagingBuffer, *buffer, bufferSize);

}

void Model::loadModel(){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())){
        throw std::runtime_error(warn + err);
    }
    for(const auto& shape: shapes){
        for(const auto& index : shape.mesh.indices){
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {1.0f, 1.0f, 1.0f};
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };
            if (uniqueVertices.count(vertex) == 0){
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            vertexIndices.push_back(uniqueVertices[vertex]);       
        }
    }
}

void Model::moveModel(glm::vec3 changeInPos){    
    modelMat = glm::translate(modelMat, changeInPos);
}

void Model::scaleModel(glm::vec3 factor){
    modelMat = glm::scale(modelMat, factor);
}

void Model::rotateModel(glm::vec3 rotation){
    modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1,0,0));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0,1,0));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0,0,1));
}

void Model::updateModelMat(){
    rotateModel(rotation);
    moveModel(velocity);
}

void Model::cleanupMemory(){
    std::cout<<"cleaning up model memory"<<std::endl;
    for(size_t j = 0; j<swapchain->swapchainImages.size(); j++){
        vkDestroyBuffer(device, uniformBuffers[j], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[j], nullptr);
    }
    vkDestroyDescriptorPool(device,*descriptorPool,nullptr);
    delete descriptorPool;
    delete descriptorSets;
    std::cout<<"finised cleaning up model memory"<<std::endl;
}

void Model::updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    //using push constants is a more efficent way to pass a small buffer of data to shaders
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currrentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currrentTime - startTime).count();
    UniformBufferObject ubo{};
    updateModelMat();
    ubo.modelView = view * modelMat;
    ubo.proj = projection;
    ubo.view = view;
    ubo.normMatrix = transpose(inverse(ubo.modelView));
    //ubo.proj[1][1] *= -1; //glm was designed for opengl where y coords are inverted so multiply by -1
    void *data;
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

void Model::setupDescriptorSetLayout(){
    auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    auto samplerLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto lightLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {lightLayoutBinding, uboLayoutBinding, samplerLayoutBinding};
    descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Model::destroyDescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete(descriptorSetLayout);
}

void Model::recreateLightBuffer(){
    createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

VkDescriptorSetLayout * Model::getDescriptorSetLayout(){
    return descriptorSetLayout;
}

std::vector<VkBuffer> * Model::getLightBuffers(){
    return &lightBuffers;
}

std::vector<VkDeviceMemory> * Model::getLightBufferMemory(){
    return &lightBuffersMemory;
}

void Model::destroyLightBufferAndMemory(size_t imageCount){
    for(size_t j = 0; j < imageCount; j++){
        vkDestroyBuffer(device, lightBuffers[j], nullptr);
        vkFreeMemory(device, lightBuffersMemory[j], nullptr);
    }
    lightBuffers.clear();
    lightBuffersMemory.clear();
}

void Model::setSwapchain(Swapchain *swapchain){
    this->swapchain = swapchain;
}