#include "vtge_descriptor.hpp"
#include "vtge_buffer_helper_functions.hpp"
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

    VkDescriptorPool *createDescriptorPool(uint32_t imageCount, std::vector<VkDescriptorPoolSize> poolSizes){
        VkDescriptorPool *descriptorPool = new VkDescriptorPool();
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = imageCount;
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

    std::vector<VkDescriptorSet> *allocateDescriptorSets(uint32_t imageCount, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool){
        std::vector<VkDescriptorSet> *descriptorSets = new std::vector<VkDescriptorSet>();
        std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = imageCount;
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets->resize(imageCount);
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

    void createDescriptorBuffer(VkDeviceSize bufferSize, std::vector<VkBuffer> *buffers, std::vector<VkDeviceMemory> *bufferMemory, VkBufferUsageFlags bufferUsage, uint32_t size){
        buffers->resize(size);
        bufferMemory->resize(size);
        for(size_t i =0; i< size; i++){
            buffer::createBuffer(bufferSize, bufferUsage, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            (*buffers)[i], (*bufferMemory)[i]);
        } 
        
    }

    void populateDescriptorBuffer(std::vector<VkDescriptorSet> * descriptorSets, uint32_t imageCount, 
        std::vector<VtgeBufferInfo> buffers, std::vector<VtgeImageInfo> images){
        for(size_t i = 0; i < imageCount; i++){
            VkDescriptorSet descriptorSet = (*descriptorSets)[i];
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            
            for(VtgeBufferInfo bInfo : buffers){
                VkDescriptorBufferInfo *bufferInfo = new VkDescriptorBufferInfo();
                bufferInfo->buffer = *bInfo.buffer;
                bufferInfo->offset = bInfo.offset;
                bufferInfo->range = bInfo.range;
                descriptorWrites.push_back(Descriptor::createWriteDescriptorSet(
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, descriptorSet, bInfo.binding,
                    0, bInfo.type, 1, bufferInfo, nullptr, nullptr));
            }
            for(VtgeImageInfo iInfo : images){
                VkDescriptorImageInfo *imageInfo = new VkDescriptorImageInfo();
                imageInfo->imageLayout = iInfo.layout;
                imageInfo->imageView = iInfo.view;
                imageInfo->sampler = iInfo.sampler;
                descriptorWrites.push_back(createWriteDescriptorSet(
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, descriptorSet, iInfo.binding,
                    0, iInfo.type, 1, nullptr, imageInfo, nullptr));
            }
            updateDescriptorSets(descriptorWrites);
            for(VkWriteDescriptorSet write : descriptorWrites){
                if(write.pBufferInfo){delete write.pBufferInfo;}
                if(write.pImageInfo){delete write.pImageInfo;}
            }
        }
    }

}