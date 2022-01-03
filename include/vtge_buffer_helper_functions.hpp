#ifndef __VTGE_BUFFER_HELPER_FUNCTIONS_HPP_
#define __VTGE_BUFFER_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "vtge_queuefamilyindices.hpp"

namespace buffer
{
    /**
     * @brief creates a buffer and allocates memory for it
     * @param size the size of the bufffer
     * @param usage a flag that indicates the buffers usage; used to create VkBufferCreateInfo
     * @param properties used to determine what kind of memory is need for this buffer
     * @param buffer a reference to the VkBuffer being created
     * @param bufferMemory a reference to the VkDeviceMemory being allocated for the buffer
     */
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    /**
     * @brief copies a buffer of a given size 
     * @param srcBuffer the source VkBuffer
     * @param dstBuffer the destination VkBuffer
     * @param size the size of the region being copied (usally the size of the srcBuffer)
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /**
     * @brief creates a buffer that is used for copying memory. stores staging buffers in a vector until they are no longer needed
     * @param size the size of the staging buffer
     * @param usage a flag that indicates the buffers usage
     * @param properties used to determine what kind of memory is needed for this buffer
     * @param buffer a refernce to the VkBuffer being created
     * @param bufferMemory a reference to the VkDeviceMemory being allocated for the buffer
     */
    void createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                             VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    /**
     * @brief destroys staging buffers stored in the vector that holds them
     */
    void cleanupStagingBuffers();
}
#endif