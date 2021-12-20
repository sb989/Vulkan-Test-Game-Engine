#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include <vtge_model.hpp>
#include <vtge_texture.hpp>
#include <array>
#include <cstring>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vtge_descriptor.hpp>
#include <vtge_ubo.hpp>
extern VkDevice device;
VkDescriptorSetLayout * Model::descriptorSetLayout = nullptr;

Model::Model(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath, std::string specularMapPath, glm::vec4 color){
    this->modelPath = modelPath;
    this->diffuseMapPath = diffuseMapPath;
    this->specularMapPath = specularMapPath;
    if(diffuseMapPath != "")
        this->diffuseMap = new Texture(diffuseMapPath);
    else if (color != glm::vec4(-1,-1,-1,-1))
        this->diffuseMap = new Texture(64, 64, color);
    else if (color == glm::vec4(-1,-1,-1,-1) && diffuseMapPath == ""){
        this->diffuseMap = new Texture(64, 64, glm::vec4(200, 200, 200, 255));
        std::cout<<modelPath<<std::endl;
    }
    if(specularMapPath != "")
        this->specularMap = new Texture(specularMapPath);
    else
        this->specularMap = new Texture(64, 64, glm::vec4(125,125,125,255));
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
    this->imageCount = imageCount;
    
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    initDescriptorSets(imageCount);
    Material mtrl = {32.0f};
    updateMaterial(mtrl);
}


Model::~Model(){
    if(diffuseMap)
        delete diffuseMap;
    if(specularMap)
        delete specularMap;
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void Model::initDescriptorSets(uint32_t imageCount){
    if(!descriptorSetLayout){
        setupDescriptorSetLayout();
    }
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
    
}

void Model::setupDescriptorSetLayout(){

    auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    auto samplerLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto specMapLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto materialLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding, samplerLayoutBinding, materialLayoutBinding, specMapLayoutBinding};

    descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Model::createDescriptorBuffers(){
    Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(sizeof(Material), &material, &materialMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
}

void Model::createDescriptorPool(){
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2*imageCount},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * imageCount}
    };
    descriptorPool = Descriptor::createDescriptorPool(imageCount, poolSizes);
}

void Model::createDescriptorSets(){
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo uniformBuffer {};
    uniformBuffer.buffer = uniformBuffers.data();
    uniformBuffer.offset = 0;
    uniformBuffer.range = sizeof(UniformBufferObject);
    uniformBuffer.binding = 0;
    uniformBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferInfos.push_back(uniformBuffer);
    VtgeBufferInfo materialBuffer {};
    materialBuffer.buffer = material.data();
    materialBuffer.offset = 0;
    materialBuffer.range = sizeof(Material);
    materialBuffer.binding = 3;
    materialBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //bufferInfos.push_back(directionalLightBuffer);
    bufferInfos.push_back(materialBuffer);
    VtgeImageInfo imageBuffer{};
    imageBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBuffer.view = diffuseMap->textureImageView;
    imageBuffer.sampler = diffuseMap->textureSampler;
    imageBuffer.binding = 1;
    imageBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VtgeImageInfo specMapBuffer{};
    specMapBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    specMapBuffer.view = specularMap->textureImageView;
    specMapBuffer.sampler = specularMap->textureSampler;
    specMapBuffer.binding = 2;
    specMapBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageInfos.push_back(imageBuffer);
    imageInfos.push_back(specMapBuffer);
    Descriptor::populateDescriptorBuffer(descriptorSets,imageCount, bufferInfos, imageInfos);
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

void Model::updateModelMat(uint32_t currentImage, glm::mat4 projection, glm::mat4 view){
    rotateModel(rotation);
    moveModel(velocity);
    UniformBufferObject ubo{};
    ubo.modelView = view * getModelMat();
    ubo.proj = projection;
    ubo.view = view;
    ubo.normMatrix = transpose(inverse(ubo.modelView));
    void *data;
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

glm::vec4 Model::getModelPos(){
    return glm::vec4(modelMat[3][0],modelMat[3][1], modelMat[3][2], 1);
}

VkDescriptorSetLayout * Model::getDescriptorSetLayout(){
    return descriptorSetLayout;
}

std::vector<VkDescriptorSet> * Model::getDescriptorSets(){
    return descriptorSets;
}

void Model::cleanupMemory(){
    for(size_t j = 0; j < imageCount; j++){
        vkDestroyBuffer(device, uniformBuffers[j], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[j], nullptr);
        vkDestroyBuffer(device, material[j], nullptr);
        vkFreeMemory(device, materialMemory[j], nullptr);
    }
    vkDestroyDescriptorPool(device,*descriptorPool,nullptr);
    delete descriptorPool;
    delete descriptorSets;
}

void Model::destroyDescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete(descriptorSetLayout);
}

void Model::recreateUBufferPoolSets(uint32_t imageCount){
    this->imageCount = imageCount;
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
    Material mtrl = {32.0f};
    updateMaterial(mtrl);
}

// void Model::recreateAllModels(uint32_t imageCount){
//     std::cout<<objectList.size()<<std::endl;
//     for(int i = 0; i < objectList.size(); i++){
//         objectList[i]->recreateUBufferPoolSets(imageCount);
//     }
// }


void Model::updateMaterial(Material mat){
    for(int currentImage = 0; currentImage < imageCount; currentImage++){
        void * data;
        vkMapMemory(device, materialMemory[currentImage], 0, sizeof(mat), 0, &data);
        memcpy(data, &mat, sizeof(mat));
        vkUnmapMemory(device, materialMemory[currentImage]);
    }
}