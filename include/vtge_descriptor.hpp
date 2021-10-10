#ifndef __VTGE_DESCRIPTOR_HPP__
#define __VTGE_DESCROPTOR_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>


struct VtgeBufferInfo{
    VkBuffer *buffer;
    VkDeviceSize offset;
    VkDeviceSize range;
    uint32_t binding;
    VkDescriptorType type; 
};

struct VtgeImageInfo{
    VkImageLayout layout;
    VkImageView view;
    VkSampler sampler;
    uint32_t binding;
    VkDescriptorType type; 
};

namespace Descriptor{
    VkDescriptorSetLayout *createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> *bindings);
    VkDescriptorPool *createDescriptorPool(uint32_t imageCount, std::vector<VkDescriptorPoolSize> poolSizes);
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(
        uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount,
        VkShaderStageFlags stageFlags, VkSampler* samplers);
    std::vector<VkDescriptorSet> *allocateDescriptorSets(uint32_t imageCount, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
    VkWriteDescriptorSet createWriteDescriptorSet(VkStructureType type, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement,
        VkDescriptorType descriptorType, uint32_t descriptorCount, VkDescriptorBufferInfo * bufferInfo, VkDescriptorImageInfo *imageInfo, VkBufferView *bufferView);
    void updateDescriptorSets(std::vector<VkWriteDescriptorSet> descriptorWrites);
    void createDescriptorBuffer(VkDeviceSize bufferSize, std::vector<VkBuffer> *buffers, std::vector<VkDeviceMemory> *bufferMemory, VkBufferUsageFlags bufferUsage, uint32_t size);
    
    
    void populateDescriptorBuffer(std::vector<VkDescriptorSet> * descriptorSets, uint32_t imageCount, 
        std::vector<VtgeBufferInfo> buffers, std::vector<VtgeImageInfo> images);
}
#endif