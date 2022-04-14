#ifndef __VTGE_DEPTH_BUFFER_HPP__
#define __VTGE_DEPTH_BUFFER_HPP__
#include <cstdint>
#include <stdint.h>
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Framebuffer;
class Mesh;
class Pipeline;
class Object;
class DepthBuffer
{
public:
    DepthBuffer(uint32_t layers, uint32_t imageCount);
    ~DepthBuffer();
    Framebuffer *getFramebuffer();
    std::vector<VkCommandBuffer> getDepthCommandBuffers();
    std::vector<VkSemaphore> getDepthSemaphore();
    VkRenderPass *getRenderPass();
    VkSampler *getSampler();
    Pipeline *getPipeline();
    void drawDepthBuffer(Object *obj, uint32_t index, uint32_t lightNum);
    VkFence fence;

private:
    uint32_t layersCount;
    uint32_t imageCount;
    Framebuffer *framebuffer;
    VkDescriptorSetLayout *depthDescriptorSetLayout;
    VkDescriptorPool *descriptorPool;
    std::vector<std::vector<VkDescriptorSet>> depthDescriptorSets;
    std::vector<VkBuffer> shadowModelLightMatBuffers, uboMeshBuffers;
    std::vector<VkDeviceMemory> shadowModelLightMatBuffersMemory, uboMeshMemory;
    std::vector<VkCommandBuffer> depthCommandBuffers;
    std::vector<VkSemaphore> renderFinishedSemaphore;
    VkRenderPass renderPass;
    Pipeline *depthPipeline;
    VkSampler depthSampler;
    void createDepthSampler();
    void setupDescriptorPool();
    void setupSemaphore();
    void setupDepthDescriptorSetLayouts();
    void createDepthDescriptorSets();
    void createDepthDescriptorBuffers();
    // void updateModelLightMat(uint32_t currentImage, ModelLightMatrix mlm);
};

#endif