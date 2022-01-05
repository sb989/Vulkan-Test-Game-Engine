#ifndef __VTGE_BUFFER_HELPER_FUNCTIONS_HPP_
#define __VTGE_BUFFER_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "vtge_queuefamilyindices.hpp"

namespace buffer
{
    //creates a buffer and allocates memory for it
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    //copies a buffer of a given size 
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    // creates a buffer that is used for copying memory. stores staging buffers in a vector until they are no longer needed
    void createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                             VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    //destroys staging buffers stored in the vector that holds them
    void cleanupStagingBuffers();
}
#endif