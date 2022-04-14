#ifndef __VTGE_GRAPHICS_HPP__
#define __VTGE_GRAPHICS_HPP__
#define GLFW_INCLUDE_VULKAN
#include "vtge_vertex.hpp"
#include "vtge_queuefamilyindices.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <optional>

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

struct PushConstants
{
    alignas(16) glm::mat4 normMatrix;
};

struct SwapchainSupportDetails;

class Swapchain;
class Framebuffer;
class Pipeline;
class Object;
class Model;
class Camera;
class Graphics
{
public:
    Graphics(uint32_t width, uint32_t height, std::string windowTitle);

    ~Graphics();

    void drawFrame();
    bool framebufferResized = false;
    GLFWwindow *window;
    static VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);
    static void beginGraphicsCommandBuffer();
    static void beginTransferCommandBuffer();
    static void endGraphicsCommandBuffer();
    static void endTransferCommandBuffer();
    static VkDevice getDevice();
    static Swapchain *getSwapchain();
    static void getSwapchainSupport(SwapchainSupportDetails *details);
    static Framebuffer *getFramebuffer();
    static VkInstance getInstance();
    static Pipeline *getGraphicsPipeline();
    static Pipeline *getLightPipeline();
    static VkCommandBuffer getGraphicsCommandBuffer();
    static VkCommandBuffer getTransferCommandBuffer();
    static std::vector<VkCommandBuffer> getDrawCommandBuffer();
    static VkQueue getGraphicsQueue();
    static VkQueue getPresentQueue();
    static VkQueue getTransferQueue();
    static VkCommandPool getGraphicsCommandPool();
    static VkCommandPool getTransferCommandPool();
    static QueueFamilyIndices getQueueFamilyIndices();
    static size_t getMaxFramesInFlight();
    static Camera *getCamera();
    static VkSurfaceKHR getSurface();
    static VkSampleCountFlagBits getMsaaSamples();
    static VkPhysicalDevice getPhysicalDevice();
    static void allocateCommandBuffer(std::vector<VkCommandBuffer> *commandBuffer, uint32_t commandBufferCount);
    static void submitQueue(VkCommandBuffer *commandBuffer, VkFence *fence, std::vector<VkSemaphore> *signalSemaphores,
                            std::vector<VkSemaphore> *waitSemaphores, std::vector<VkPipelineStageFlags> *waitStages);

    static void beginCommandBuffer(VkCommandBuffer cmdBuffer, std::string name);
    static void endCommandBuffer(VkCommandBuffer cmdBuffer);

private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;

    std::string windowTitle = "Vulkan Test Game Engine - FPS: ";
    float frameCount = 0;

    float xVel, yVel, zVel;
    uint32_t mipLevels;
    uint32_t WIDTH, HEIGHT;
    static std::vector<VkCommandBuffer> drawCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores, renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences, imagesInFlight;
    static Swapchain *swapchain;
    static Camera *cam;
    static SwapchainSupportDetails *swapchainSupport;
    static Framebuffer *framebuffer, *shadowFramebuffer;
    static VkInstance instance;
    static VkRenderPass renderPass;
    static Pipeline *graphicsPipeline, *lightPipeline;
    static VkQueue graphicsQueue, presentQueue, transferQueue;
    static VkCommandPool graphicsCommandPool, transferCommandPool;
    static QueueFamilyIndices indices;
    static VkCommandBuffer transferCommandBuffer, graphicsCommandBuffer;
    VkDebugUtilsMessengerEXT debugMessenger;
    static VkSurfaceKHR surface;
    static VkDevice device;
    void setUpWindow();

    void setUpGraphics();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createCommandPool();

    void allocateCommandBuffers();
    void populateAndDrawObjects(uint32_t imageIndex);

    void populateDrawCommandBuffer(size_t index);

    void createSyncObjects();

    bool waitForFence(uint32_t &imageIndex);

    void presentQueueToScreen(uint32_t &imageIndex, std::vector<VkSemaphore> *signalSemaphores);

    void recreateSwapchain();

    void cleanupSwapchain();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void querySwapchainSupport(VkPhysicalDevice testDevice, SwapchainSupportDetails *details);
};
#endif