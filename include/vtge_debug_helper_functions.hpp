#ifndef __VTGE_DEBUG_HELPER_FUNCTIONS_HPP__
#define __VTGE_DEBUG_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

namespace debug{
    //setups the debug messenger
    void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger);

    //creates a debug messenger 
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    //destroys a debug messenger
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger, const VkAllocationCallbacks* pAllocator);
    
    //populates the debug messengers create info struct
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // a callback function for debugging
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

};

#endif