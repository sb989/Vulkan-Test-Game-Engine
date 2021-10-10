#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include <vtge_model.hpp>
#include <vtge_texture.hpp>
#include <array>
#include <cstring>
#include <tiny_obj_loader.h>
#include <unordered_map>
extern VkDevice device;
//extern VkDescriptorSetLayout descriptorSetLayout;
Model::Model(std::string modelPath, std::string texturePath){
    this->modelPath = modelPath;
    this->texturePath = texturePath;
    //this->imageCount = imageCount;
    this->texture = new Texture(texturePath);
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
    // if(!descriptorSetLayout){
    //     setupDescriptorSetLayout();
    //     Descriptor::createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    // }
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    // createUniformBuffers();
    // createDescriptorPool();
    // createDescriptorSets();
}

Model::Model(std::string modelPath){
    this->modelPath = modelPath;
    //this->imageCount = imageCount;
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
    this->texture = NULL;
    // if(!descriptorSetLayout){
    //     setupDescriptorSetLayout();
    //     Descriptor::createDescriptorBuffer(sizeof(LightInfo) * MAX_LIGHT_COUNT, &lightBuffers, &lightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    // }
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    // createUniformBuffers();
    // createDescriptorPool();
    // createDescriptorSets();
}

Model::~Model(){
    if(texture)
        delete texture;
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
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

glm::vec3 Model::getModelPos(){
    return glm::vec3(modelMat[3][0],modelMat[3][1], modelMat[3][2]);
}

