#ifndef __VTGE_DEBUG_HELPER_FUNCTIONS_HPP__
#define __VTGE_DEBUG_HELPER_FUNCTIONS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

namespace debug{
    /**
     * @brief setups the debug messenger
     * @param instance the vulkan instance currently being used
     * @param debugMessenger pointer to the debug messenger being setup
     */
    void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger);

    /**
     * @brief creates a debug messenger 
     * @param instance the vulkan instance currently being used
     * @param pCreateInfo a pointer to the createinfo struct for the debug messenger being created
     * @param pAllocator a pointer to the allocation callback
     * @param pDebugMessenger a pointer to the debug messenger beign created
     * @return a VkResult object; VK_SUCCESS or something else
     */
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    /**
     * @brief destroys a debug messenger
     * @param instance the vulkan instance currently being used
     * @param debugMessenger a pointer to the debug messenger that needs to be destroyed
     * @param pAllocator a pointer to the debug messengers allocator callback
     */
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger, const VkAllocationCallbacks* pAllocator);
    
    /**
     * @brief populates the debug messengers create info struct
     * @param createInfo a reference to the create info struct for the debug messenger
     */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
     * @brief a callback function for debugging
     * @param messageSeverity a flag that indicates the severity of the message
     * @param messageType a flag that indicates the message type
     * @param pCallbackData a pointer to the callback data
     * @param pUserData a pointer to any user data that needs to be returned
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

}  

#endif