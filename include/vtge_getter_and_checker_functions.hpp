#ifndef __VTGE_GETTER_CHECKER_HPP__
#define __VTGE_GETTER_CHECKER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include "vtge_queuefamilyindices.hpp"
struct SwapchainSupportDetails
{
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    VkSurfaceCapabilitiesKHR capabilities;
};
struct getterChecker
{
    static float SHADOW_MAP_HEIGHT;
    static float SHADOW_MAP_WIDTH;
    // finds the memory type needed for the properties provided. filter removes memory types not needed
    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // determines if the physical device matches the requirements
    static bool isDeviceSuitable(VkPhysicalDevice device, SwapchainSupportDetails swapChainSupport, std::vector<const char *> deviceExtensions);

    // determines if the physical device supports the list of device extensions
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> deviceExtensions);

    // determines if the validation layers are supported
    static bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

    static std::vector<const char *> getRequiredExtensions();

    static std::vector<char> readFile(const std::string &filename);

    static VkFormat findDepthFormat();

    static VkSampleCountFlagBits getMaxUsableSampleCount();

    static VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                        VkFormatFeatureFlags features);

    static float getShadowMapHeight();
    static float getShadowMapWidth();
};

#endif