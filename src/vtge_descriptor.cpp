#include "vtge_descriptor.hpp"
#include "vtge_swapchain.hpp"
#include <stdexcept>
extern VkDevice device;
namespace Descriptor{
    VkDescriptorSetLayout *createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> *bindings){            
        VkDescriptorSetLayout *descriptorSetLayout = new VkDescriptorSetLayout();
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings->size());
        layoutInfo.pBindings = bindings->data();
        if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, descriptorSetLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return descriptorSetLayout;
    }

    VkDescriptorPool *createDescriptorPool(Swapchain *swapchain, std::vector<VkDescriptorPoolSize> poolSizes){
        VkDescriptorPool *descriptorPool = new VkDescriptorPool();
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(swapchain->swapchainImages.size());
        if(vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool) != VK_SUCCESS){
            throw std::runtime_error("failed to create descriptor pool!");
        }
        return descriptorPool;
    }
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(
        uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount,
        VkShaderStageFlags stageFlags, VkSampler* samplers){
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorType = descriptorType;
            layoutBinding.descriptorCount = descriptorCount;
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = samplers;
            return layoutBinding;
    }

    std::vector<VkDescriptorSet> *allocateDescriptorSets(Swapchain *swapchain, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool){
        std::vector<VkDescriptorSet> *descriptorSets = new std::vector<VkDescriptorSet>();
        std::vector<VkDescriptorSetLayout> layouts(swapchain->swapchainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain->swapchainImages.size());
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets->resize(swapchain->swapchainImages.size());
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets->data()) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate descriptor sets");
        }
        return descriptorSets;
    }

    VkWriteDescriptorSet createWriteDescriptorSet(VkStructureType type, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement,
        VkDescriptorType descriptorType, uint32_t descriptorCount, VkDescriptorBufferInfo * bufferInfo, VkDescriptorImageInfo *imageInfo, VkBufferView *bufferView){
        
        VkWriteDescriptorSet descriptorWrites{};
        descriptorWrites.sType = type;
        descriptorWrites.dstSet = descriptorSet;
        descriptorWrites.dstBinding = binding;
        descriptorWrites.dstArrayElement = arrayElement;
        descriptorWrites.descriptorType = descriptorType;
        descriptorWrites.descriptorCount = descriptorCount;
        descriptorWrites.pBufferInfo = bufferInfo;
        descriptorWrites.pImageInfo = imageInfo;
        descriptorWrites.pTexelBufferView = bufferView;
        return descriptorWrites;
    }

    void updateDescriptorSets(std::vector<VkWriteDescriptorSet> descriptorWrites){
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }

    

}