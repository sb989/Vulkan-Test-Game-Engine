#ifndef __VTGE_DESCRIPTOR_HPP__
#define __VTGE_DESCROPTOR_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
class Swapchain;

namespace Descriptor{
    VkDescriptorSetLayout *createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> *bindings);
    VkDescriptorPool *createDescriptorPool(Swapchain *swapchain, std::vector<VkDescriptorPoolSize> poolSizes);
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(
        uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount,
        VkShaderStageFlags stageFlags, VkSampler* samplers);
    std::vector<VkDescriptorSet> *allocateDescriptorSets(Swapchain *swapchain, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
    VkWriteDescriptorSet createWriteDescriptorSet(VkStructureType type, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement,
        VkDescriptorType descriptorType, uint32_t descriptorCount, VkDescriptorBufferInfo * bufferInfo, VkDescriptorImageInfo *imageInfo, VkBufferView *bufferView);
    void updateDescriptorSets(std::vector<VkWriteDescriptorSet> descriptorWrites);
}

#endif