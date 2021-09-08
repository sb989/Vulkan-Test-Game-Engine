#ifndef __VTGE_BUFFER_HELPER_FUNCTIONS_HPP_
#define __VTGE_BUFFER_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include <vector>
#include "vtge_queuefamilyindices.hpp"

namespace buffer{
    
    
    
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void cleanupStagingBuffers();

}
#endif