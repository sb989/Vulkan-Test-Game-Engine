#ifndef __VTGE_GRAPHICS_HPP__
#define __VTGE_GRAPHICS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <GLFW/glfw3.h>
#include <vector>
#include "vtge_vertex.hpp"
#include <optional>
#include "vtge_model.hpp"
#include "vtge_getter_and_checker_functions.hpp"
class Swapchain;
class Framebuffer;

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Graphics{
    public:
        Graphics(uint32_t width, uint32_t height, std::string windowTitle);
        ~Graphics();
        void                            drawFrame();
        bool                            framebufferResized = false;
        GLFWwindow                      *window;
    private:
        
        const int                       MAX_FRAMES_IN_FLIGHT = 2;
        size_t                          currentFrame = 0;
        const std::string               VIKING_MODEL_PATH = "../models/viking_room.obj";
        const std::string               VIKING_TEXTURE_PATH = "../textures/viking_room.png";
        const std::string               BANANA_MODEL_PATH = "../models/ripe-banana.obj";
        const std::string               BANANA_TEXTURE_PATH = "../textures/ripe-banana_u1_v1.png";
        std::string                     windowTitle = "Vulkan Test Game Engine - FPS: ";
        float                           frameCount = 0;
        float                           camXPos, camYPos, camZPos;
        float                           xVel, yVel, zVel;
        uint32_t                        mipLevels;
        uint32_t                        WIDTH, HEIGHT;
        std::vector<VkCommandBuffer>    drawCommandBuffers;
        std::vector<VkSemaphore>        imageAvailableSemaphores, renderFinishedSemaphores;
        std::vector<VkFence>            inFlightFences, imagesInFlight;
        std::vector<Model*>             modelList;
        Swapchain                       *swapchain;
        SwapchainSupportDetails         swapchainSupport;
        Framebuffer                     *framebuffer;
        VkInstance                      instance;
        VkRenderPass                    renderPass;
        VkPipelineLayout                pipelineLayout;
        VkPipeline                      graphicsPipeline;
        VkDescriptorPool                descriptorPool;        
        VkDebugUtilsMessengerEXT        debugMessenger;
        VkSurfaceKHR                    surface;

        void setUpWindow();
        void setUpGraphics();
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void initWindow();
        void createLogicalDevice();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createColorResources();
        void createDepthResources();
        void createDrawCommandBuffers();
        void createSyncObjects();
        void handleKeyPress(GLFWwindow* window);
        void updateUniformBuffer(uint32_t currentImage, Model *m);
        void createModel(std::string modelPath, std::string texturePath);
        void recreateSwapchain();
        void cleanupSwapchain();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);
        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice testDevice);


};

#endif