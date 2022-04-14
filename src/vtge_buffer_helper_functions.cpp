#include "vtge_buffer_helper_functions.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_graphics.hpp"
std::vector<VkDeviceMemory> stagingBuffersMemory = {0};
std::vector<VkBuffer> stagingBuffers = {0};

void buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                          VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    QueueFamilyIndices indices = Graphics::getQueueFamilyIndices();
    VkDevice device = Graphics::getDevice();
    uint32_t queueIndices[] = {indices.graphicsFamily.value(), indices.transferFamily.value()};
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    bufferInfo.queueFamilyIndexCount = 2;
    bufferInfo.pQueueFamilyIndices = queueIndices;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getterChecker::findMemoryType(memRequirements.memoryTypeBits,
                                                              properties);

    /***
    TODO: replace vkAllocateMemory with either a custom memory allocator or use VulkanMemoryAllocator
    ***/
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer transferCommandBuffer = Graphics::getTransferCommandBuffer();
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void buffer::createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    createBuffer(size, usage, properties, buffer, bufferMemory);
    stagingBuffersMemory.push_back(bufferMemory);
    stagingBuffers.push_back(buffer);
}

void buffer::cleanupStagingBuffers()
{
    VkDevice device = Graphics::getDevice();
    int stagingBufferCount = stagingBuffersMemory.size();
    for (int i = 0; i < stagingBufferCount; i++)
    {
        vkDestroyBuffer(device, stagingBuffers[i], nullptr);
        vkFreeMemory(device, stagingBuffersMemory[i], nullptr);
    }
}