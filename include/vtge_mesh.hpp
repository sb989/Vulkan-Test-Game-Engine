#ifndef __VTGE_MESH_HPP__
#define __VTGE_MESH_HPP__

#include "vtge_vertex.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
struct Material
{
    alignas(4) float shininess;
};
struct UniformBufferObject;
struct Node
{
    struct Node *parent;
    aiMatrix4x4 transform;
};

class Texture;

class Mesh
{
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
         std::vector<Texture *> diffuseMap, std::vector<Texture *> specularMap, uint32_t imageCount, Node *n);
    ~Mesh();
    void recreateUBufferPoolSets(uint32_t imageCount);
    static VkDescriptorSetLayout *getDescriptorSetLayout();
    std::vector<VkDescriptorSet> *getDescriptorSets();
    static void destroyDescriptorSetLayout();
    void drawMesh(std::vector<VkDescriptorSet> combinedDescriptorSets, VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index);
    void updateMaterial(Material mat);
    void cleanupMemory();
    void updateUniformBuffers(UniformBufferObject ubo, uint32_t currentImage);
    static glm::mat4 assimpMat4ToGlmMat4(aiMatrix4x4 mat4);
    static void initMeshSystem();

private:
    static VkDescriptorSetLayout *descriptorSetLayout;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture *> diffuseMap;
    std::vector<Texture *> specularMap;
    std::vector<VkDeviceMemory> uniformBuffersMemory, materialMemory;
    std::vector<VkBuffer> uniformBuffers, material;
    std::vector<VkDescriptorSet> *descriptorSets;
    VkDescriptorPool *descriptorPool;
    VkBuffer vertexBuffer, indexBuffer;
    VkDeviceMemory vertexBufferMemory, indexBufferMemory;
    Node *node;
    uint32_t imageCount;

    void createVertexBuffer();

    void createIndexBuffer();
    void createBufferAndCopy(VkDeviceSize bufferSize, VkBuffer *buffer, VkDeviceMemory *deviceMemory, VkBufferUsageFlags flags, void *pointer);
    void createDescriptorSetLayout();

    static void setupDescriptorSetLayout();

    void initDescriptorSets(uint32_t imageCount);

    void createDescriptorBuffers();

    void createDescriptorPool();

    void createDescriptorSets();

    glm::mat4 getMeshTransform();
};

#endif