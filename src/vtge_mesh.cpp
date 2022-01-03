#include "vtge_mesh.hpp"
#include <vtge_descriptor.hpp>
#include "vtge_buffer_helper_functions.hpp"
#include "vtge_ubo.hpp"
#include <cstring>
#include "vtge_texture.hpp"
#include "vtge_light.hpp"

extern VkDevice device;
VkDescriptorSetLayout *Mesh::descriptorSetLayout = nullptr;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture *> diffuseMap, std::vector<Texture *> specularMap, uint32_t imageCount, Node *n)
{
    this->vertices = vertices;
    this->indices = indices;
    this->diffuseMap = diffuseMap;
    this->specularMap = specularMap;
    this->imageCount = imageCount;
    this->node = n;
    createVertexBuffer();
    createIndexBuffer();
    initDescriptorSets(imageCount);
    Material mtrl = {32.0f};
    updateMaterial(mtrl);
}

Mesh::~Mesh()
{
    // for(int i = 0; i < diffuseMap.size(); i++){
    //     delete diffuseMap[i];
    // }
    // for(int i = 0; i < specularMap.size(); i++){
    //     delete specularMap[i];
    // }
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    delete node;
}

void Mesh::drawMesh(std::vector<VkDescriptorSet> combinedDescriptorSets, VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index)
{
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    //VkDescriptorSet combindedDescriptorSets [2] = {(*descriptorSets)[index], (*Light::getDescriptorSets())[index]};
    combinedDescriptorSets.insert(combinedDescriptorSets.begin(), (*descriptorSets)[index]);
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, 0, combinedDescriptorSets.size(), combinedDescriptorSets.data(), 0, nullptr);
    vkCmdDrawIndexed(*commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

glm::mat4 Mesh::getMeshTransform()
{
    Node *currentNode = this->node;
    aiMatrix4x4 transform = aiMatrix4x4();
    while (currentNode != nullptr)
    {
        transform = currentNode->transform * transform;
        currentNode = currentNode->parent;
    }
    return assimpMat4ToGlmMat4(transform);
}

glm::mat4 Mesh::assimpMat4ToGlmMat4(aiMatrix4x4 aiMat)
{
    glm::mat4 ret = {
        aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
        aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
        aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
        aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4};
    return ret;
}

void Mesh::initDescriptorSets(uint32_t imageCount)
{
    if (!descriptorSetLayout)
    {
        setupDescriptorSetLayout();
    }
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Mesh::createDescriptorBuffers()
{
    Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(sizeof(Material), &material, &materialMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
}

void Mesh::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * imageCount},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * imageCount}};
    descriptorPool = Descriptor::createDescriptorPool(imageCount, poolSizes);
}

void Mesh::setupDescriptorSetLayout()
{

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

void Mesh::createDescriptorSets()
{
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo uniformBuffer{};
    uniformBuffer.buffer = uniformBuffers.data();
    uniformBuffer.offset = 0;
    uniformBuffer.range = sizeof(UniformBufferObject);
    uniformBuffer.binding = 0;
    uniformBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferInfos.push_back(uniformBuffer);
    VtgeBufferInfo materialBuffer{};
    materialBuffer.buffer = material.data();
    materialBuffer.offset = 0;
    materialBuffer.range = sizeof(Material);
    materialBuffer.binding = 3;
    materialBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //bufferInfos.push_back(directionalLightBuffer);
    bufferInfos.push_back(materialBuffer);
    VtgeImageInfo imageBuffer{};
    imageBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBuffer.view = diffuseMap[0]->textureImageView;
    imageBuffer.sampler = diffuseMap[0]->textureSampler;
    imageBuffer.binding = 1;
    imageBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VtgeImageInfo specMapBuffer{};
    specMapBuffer.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    specMapBuffer.view = specularMap[0]->textureImageView;
    specMapBuffer.sampler = specularMap[0]->textureSampler;
    specMapBuffer.binding = 2;
    specMapBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageInfos.push_back(imageBuffer);
    imageInfos.push_back(specMapBuffer);
    Descriptor::populateDescriptorBuffer(descriptorSets, imageCount, bufferInfos, imageInfos);
}

void Mesh::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBufferAndCopy(bufferSize, &vertexBuffer, &vertexBufferMemory,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.data());
}

void Mesh::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    createBufferAndCopy(bufferSize, &indexBuffer, &indexBufferMemory,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.data());
}

void Mesh::createBufferAndCopy(VkDeviceSize bufferSize, VkBuffer *buffer, VkDeviceMemory *deviceMemory, VkBufferUsageFlags flags, void *pointer)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    buffer::createStagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pointer, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    buffer::createBuffer(bufferSize, flags,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *deviceMemory);
    buffer::copyBuffer(stagingBuffer, *buffer, bufferSize);
}

VkDescriptorSetLayout *Mesh::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

std::vector<VkDescriptorSet> *Mesh::getDescriptorSets()
{
    return descriptorSets;
}

void Mesh::recreateUBufferPoolSets(uint32_t imageCount)
{
    this->imageCount = imageCount;
    createDescriptorBuffers();
    createDescriptorPool();
    createDescriptorSets();
    Material mtrl = {32.0f};
    updateMaterial(mtrl);
}

void Mesh::updateMaterial(Material mat)
{
    for (int currentImage = 0; currentImage < imageCount; currentImage++)
    {
        void *data;
        vkMapMemory(device, materialMemory[currentImage], 0, sizeof(mat), 0, &data);
        memcpy(data, &mat, sizeof(mat));
        vkUnmapMemory(device, materialMemory[currentImage]);
    }
}

void Mesh::destroyDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete (descriptorSetLayout);
}

void Mesh::cleanupMemory()
{
    for (size_t j = 0; j < imageCount; j++)
    {
        vkDestroyBuffer(device, uniformBuffers[j], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[j], nullptr);
        vkDestroyBuffer(device, material[j], nullptr);
        vkFreeMemory(device, materialMemory[j], nullptr);
    }
    vkDestroyDescriptorPool(device, *descriptorPool, nullptr);
    delete descriptorPool;
    delete descriptorSets;
}

void Mesh::updateUniformBuffers(UniformBufferObject ubo, uint32_t currentImage)
{
    void *data;
    ubo.modelView = ubo.modelView * getMeshTransform();
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}