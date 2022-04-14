#ifndef __VTGE_IMAGE_HPP__
#define __VTGE_IMAGE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace image
{

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                     VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage &image, VkDeviceMemory &imageMemory, uint32_t layers = 1);

    bool hasStencilComponent(VkFormat format);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkCommandBuffer commandBuffer, uint32_t mipLevels, uint32_t layerCount);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels, uint32_t baseLayer = 0, uint32_t layerCount = 1, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

}

#endif