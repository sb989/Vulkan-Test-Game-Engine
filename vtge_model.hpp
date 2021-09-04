#ifndef __VTGE_MODEL_HPP__
#define __VTGE_MODEL_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "vtge_texture.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_buffer_helper_functions.hpp"
#include <vector>
#include "vtge_vertex.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
            (hash<glm::vec3> () (vertex.color) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Model{
    public:
        Model(std::string modelPath, std::string texturePath, Swapchain *swapchain);
        void recreateUBufferPoolSets(Swapchain *swapchain);
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        VkBuffer vertexBuffer, indexBuffer;
        VkDeviceMemory vertexBufferMemory, indexBufferMemory;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> vertexIndices;
    private:
        std::string modelPath, texturePath;
        Texture *texture;
        Swapchain *swapchain;
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        std::vector<VkDescriptorSet> descriptorSets;
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        //void copyBufferToImage();
        void createVertexBuffer();
        void createIndexBuffer();
        void loadModel();
        //void createTextureImage();
};

#endif