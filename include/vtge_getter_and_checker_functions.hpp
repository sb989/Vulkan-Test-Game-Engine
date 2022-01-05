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
namespace getterChecker
{
    //finds the memory type needed for the properties provided. filter removes memory types not needed
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // determines if the physical device matches the requirements
    bool isDeviceSuitable(VkPhysicalDevice device, SwapchainSupportDetails swapChainSupport, std::vector<const char *> deviceExtensions);

    //determines if the physical device supports the list of device extensions
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> deviceExtensions);

    //determines if the validation layers are supported
    bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

    std::vector<const char *> getRequiredExtensions();

    std::vector<char> readFile(const std::string &filename);

    VkFormat findDepthFormat();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
}

#endif