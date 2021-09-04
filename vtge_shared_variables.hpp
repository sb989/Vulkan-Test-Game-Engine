#ifndef __VTGE_SHARED_VARIABLES_HPP__
#define __VTGE_SHARED_VARIABLES_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vtge_queuefamilyindices.hpp"
    namespace sharedVariables{
        VkDevice *device;
        VkPhysicalDevice *physicalDevice;
        VkSampleCountFlagBits msaaSamples;
        VkQueue *transferQueue, *graphicsQueue;
        VkCommandPool *transferCommandPool, *graphicsCommandPool;
        VkCommandBuffer *transferCommandBuffer, *graphicsCommandBuffer;
        QueueFamilyIndices indices;
        bool enableValidationLayers;
    }
    
#endif