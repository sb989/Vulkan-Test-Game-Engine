#ifndef __VTGE_IMAGE_HPP__
#define __VTGE_IMAGE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace image
{

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                     VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage &image, VkDeviceMemory &imageMemory);

    bool hasStencilComponent(VkFormat format);


    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkCommandBuffer commandBuffer, uint32_t mipLevels);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

}

#endif