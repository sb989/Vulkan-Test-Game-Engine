#ifndef __VTGE_GETTER_CHECKER_HPP__
#define __VTGE_GETTER_CHECKER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include "vtge_queuefamilyindices.hpp"
//#include "vtge_swapchain.hpp"
struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
namespace getterChecker{
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findDepthFormat();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    bool isDeviceSuitable(VkPhysicalDevice device, SwapchainSupportDetails swapChainSupport, std::vector<const char*> deviceExtensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);
    bool checkValidationLayerSupport(std::vector<const char*> validationLayers );
    std::vector<const char*> getRequiredExtensions();
    std::vector<char> readFile(const std::string& filename);
}

#endif