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
    /**
     * @brief finds the memory type needed for the properties provided
     * @param typeFilter filter to remove  memory types that arent wanted
     * @param properties properties that the memory needs to have
     * @return returns a uint32_t that specifies the memory type for the buffer
     */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
     * @brief determines if the physical device is matches the requirements
     * @param device the physical device that will be checked
     * @param swapchainSupport supported features of the physical device being checled
     * @param deviceExtensions a list of device extensions that must be supported
     * @return returns true or false depending on if the device is suitable or not
     */
    bool isDeviceSuitable(VkPhysicalDevice device, SwapchainSupportDetails swapChainSupport, std::vector<const char *> deviceExtensions);

    /**
     * @brief determines if the physical device supports the list of device extensions
     * @param device the physical device that is being checked
     * @param deviceExtensions the list of device extensions that must be supported
     * @return returns true if the device supports the device extensions and false if not
     */
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> deviceExtensions);

    /**
     * @brief determines if the validation layers are supported
     * @param validationLayers a list of validation layers to check for suppory
     * @return returns true if all layers are supported; false otherwise
     */
    bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

    /**
     * @brief finds required extensions 
     * @return returns a list of required extensions
     */
    std::vector<const char *> getRequiredExtensions();

    /**
     * @brief reads a file and returns the contents
     * @param filename the name of the file being read
     * @return a vector of chars of the contents of the file
     */
    std::vector<char> readFile(const std::string &filename);

    /**
     * @brief find the depth format that supports a list of features
     * @return returns a vkformat that supports certain features
     */
    VkFormat findDepthFormat();

    /**
     * @brief finds the max usable sample count for the physical device
     * @return returns a VkSampleCountFlagBits object that determines the max sample count
     */
    VkSampleCountFlagBits getMaxUsableSampleCount();

    /**
     * @brief finds a vkformat that supports certain criteria
     * @param candidates a list of vkformats to consider when finding a format that is supported
     * @param tiling a VkImageTiling object that determines the type of tiling used
     * @param features a VkFormatFeatureFlags object that is used to determine if a candidate supports features the user wants
     * @return returns a VkFormat object that supports the features or throws a runtime error
     */
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
}

#endif