#ifndef __VTGE_BUFFER_HELPER_FUNCTIONS_HPP_
#define __VTGE_BUFFER_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include "vtge_queuefamilyindices.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_shared_variables.hpp"
namespace buffer{

    std::vector<VkDeviceMemory> stagingBuffersMemory = {0};
    std::vector<VkBuffer> stagingBuffers = {0};
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory){
        uint32_t queueIndices []= {sharedVariables::indices.graphicsFamily.value(), sharedVariables::indices.transferFamily.value()};
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2;
        bufferInfo.pQueueFamilyIndices = queueIndices;
        if (vkCreateBuffer(*sharedVariables::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
            throw std::runtime_error("failed to create buffer!");
        }
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*sharedVariables::device, buffer, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = getterChecker::findMemoryType(memRequirements.memoryTypeBits,
            properties);

        //replace vkAllocateMemory with either a custom memory allocator or use VulkanMemoryAllocator
        if (vkAllocateMemory(*sharedVariables::device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        vkBindBufferMemory(*sharedVariables::device, buffer, bufferMemory, 0);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
        //VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(*sharedVariables::transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        //endSingleTimeCommands(commandBuffer, transferCommandPool, transferQueue);
    }

    void createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory){
        createBuffer(size, usage, properties, buffer, bufferMemory);
        stagingBuffersMemory.push_back(bufferMemory);
        stagingBuffers.push_back(buffer);
    }

    void cleanupStagingBuffers(){
        int stagingBufferCount = stagingBuffersMemory.size();
        for(int i = 0; i < stagingBufferCount; i++){
            vkDestroyBuffer(*sharedVariables::device, stagingBuffers[i], nullptr);
            vkFreeMemory(*sharedVariables::device, stagingBuffersMemory[i], nullptr);
        }
    }

}
#endif