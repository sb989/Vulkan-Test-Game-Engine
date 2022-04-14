#ifndef __VTGE_RENDERPASS_HPP__
#define __VTGE_RENDERPASS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>
class Swapchain;
struct RenderPass
{
    static void createDisplayRenderPass(VkRenderPass *renderPass, VkSampleCountFlagBits msaaSamples, Swapchain *swapchain);
    static void createShadowMapRenderPass(VkRenderPass *renderPass, VkSampleCountFlagBits msaaSamples);
    static void startDisplayRenderPass(VkRenderPass *renderPass, VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer drawCommandBuffer);
    static void startShadowRenderPass(VkRenderPass *renderPass, VkFramebuffer framebuffer, VkCommandBuffer drawCommandBuffer);
    static void startRenderPass(VkRenderPass *renderPass, VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer drawCommandBuffer, std::vector<VkClearValue> clearValues);
};

#endif