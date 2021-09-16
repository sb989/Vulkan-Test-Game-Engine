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
extern VkDevice device;
extern VkDescriptorSetLayout descriptorSetLayout;


Model::Model(std::string modelPath, std::string texturePath, Swapchain *swapchain){
    this->modelPath = modelPath;
    this->texturePath = texturePath;
    this->swapchain = swapchain;
    this->texture = new Texture(texturePath);
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
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

void Model::createUniformBuffers(){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(swapchain->swapchainImages.size());
    uniformBuffersMemory.resize(swapchain->swapchainImages.size());

    for(size_t i =0; i<swapchain->swapchainImages.size(); i++){
        buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i], uniformBuffersMemory[i]);
    }

    VkDeviceSize lightBufferSize = sizeof(LightInfo);
    lightBuffers.resize(swapchain->swapchainImages.size());
    lightBuffersMemory.resize(swapchain->swapchainImages.size());

    for(size_t i =0; i<swapchain->swapchainImages.size(); i++){
        buffer::createBuffer(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        lightBuffers[i], lightBuffersMemory[i]);
    }
}

void Model::createDescriptorPool(){
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapchain->swapchainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapchain->swapchainImages.size());
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(swapchain->swapchainImages.size());
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapchain->swapchainImages.size());
    if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS){
        throw std::runtime_error("failed to create descriptor pool!");
    }            
}

void Model::createDescriptorSets(){
    std::vector<VkDescriptorSetLayout> layouts(swapchain->swapchainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain->swapchainImages.size());
    allocInfo.pSetLayouts = layouts.data();
    descriptorSets.resize(swapchain->swapchainImages.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS){
        throw std::runtime_error("failed to allocate descriptor sets");
    }

    for(size_t i = 0; i < swapchain->swapchainImages.size(); i++){
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightBuffers[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(LightInfo);
        if(texture){
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->textureImageView;
            imageInfo.sampler = texture->textureSampler;
            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pTexelBufferView = nullptr;
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &lightBufferInfo;
            descriptorWrites[2].pImageInfo = nullptr;
            descriptorWrites[2].pTexelBufferView = nullptr;
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
        } else{
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pTexelBufferView = nullptr;
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 2;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pBufferInfo = &lightBufferInfo;
            descriptorWrites[1].pImageInfo = nullptr;
            descriptorWrites[1].pTexelBufferView = nullptr;
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
        }        
    }
}

void Model::createVertexBuffer(){
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    buffer::createStagingBuffer(bufferSize,  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    buffer::createBuffer(bufferSize,  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer, vertexBufferMemory);
    buffer::copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void Model::createIndexBuffer(){

    VkDeviceSize bufferSize = sizeof(vertexIndices[0]) * vertexIndices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    buffer::createStagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertexIndices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    buffer::copyBuffer(stagingBuffer, indexBuffer, bufferSize);
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
    moveModel(velocity);
    rotateModel(rotation);
    //std::cout<<"rotation is :"<<glm::to_string(rotation)<<std::endl;
    //std::cout<<"modelMat is :"<<glm::to_string(modelMat)<<std::endl;
}