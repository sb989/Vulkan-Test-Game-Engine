#ifndef __VTGE_GRAPHICS_HPP__
#define __VTGE_GRAPHICS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <GLFW/glfw3.h>
#include <vector>
#include "vtge_vertex.hpp"
#include <optional>
#include "vtge_queuefamilyindices.hpp"
#include "vtge_model.hpp"
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
        Graphics(uint32_t width, uint32_t height,
            bool enableValidationLayers, std::string windowTitle);
        ~Graphics();
        GLFWwindow                      *window;
        void drawFrame();
    private:
        
        const int                       MAX_FRAMES_IN_FLIGHT = 2;
        bool                            framebufferResized = false;
        size_t                          currentFrame = 0;
        const std::string               MODEL_PATH = "models/viking_room.obj";//"models/ripe-banana.obj";
        const std::string               TEXTURE_PATH = "textures/viking_room.png";//textures/ripe-banana_u1_v1.png";
        float                           frameCount = 0;
        std::string                     windowTitle = "Vulkan Test Game Engine - FPS: ";
        float                           camXPos, camYPos, camZPos;
        float                           xVel, yVel, zVel;
        uint32_t                        mipLevels;
        uint32_t                        WIDTH, HEIGHT;
        bool                            enableValidationLayers;
        VkInstance                      instance;
        
        VkRenderPass                    renderPass;
        VkDescriptorSetLayout           descriptorSetLayout;
        VkPipelineLayout                pipelineLayout;
        VkPipeline                      graphicsPipeline;
        VkDescriptorPool                descriptorPool;
        VkCommandBuffer                 graphicsCommandBuffer, transferCommandBuffer;
        
        Swapchain                       *swapchain;
        Framebuffer                     *framebuffer;
        VkDebugUtilsMessengerEXT        debugMessenger;
        VkSurfaceKHR                    surface;

        std::vector<VkDescriptorSet>    descriptorSets;
        ///std::vector<VkBuffer>           uniformBuffers;
        //std::vector<VkDeviceMemory>     uniformBuffersMemory;

        std::vector<VkCommandBuffer>    drawCommandBuffers;
        std::vector<VkSemaphore>        imageAvailableSemaphores, renderFinishedSemaphores;
        std::vector<VkFence>            inFlightFences, imagesInFlight;

        std::vector<Model*>             modelList;

        
        void setUpWindow();
        void setUpGraphics();
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void initWindow();
        void createLogicalDevice();
        //void createSwapChain();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createColorResources();
        void createDepthResources();
        //void createFramebuffers();
        /*void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void loadModel();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        */
        void createDrawCommandBuffers();
        void createSyncObjects();
        void handleKeyPress(GLFWwindow* window);
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void updateUniformBuffer(uint32_t currentImage, Model *m);
        void createModel(std::string modelPath, std::string texturePath);
        void recreateSwapchain();
        void cleanupSwapchain();
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);


};

#endif