#ifndef __VTGE_GRAPHICS_HPP__
#define __VTGE_GRAPHICS_HPP__
#define GLFW_INCLUDE_VULKAN
#include "vtge_vertex.hpp"
#include "vtge_getter_and_checker_functions.hpp"
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
    static SwapchainSupportDetails getSwapchainSupport();
    static Framebuffer *getFramebuffer();
    static VkInstance getInstance();
    static Pipeline *getGraphicsPipeline();
    static Pipeline *getLightPipeline();
    static VkCommandBuffer getGraphicsCommandBuffer();
    static VkCommandBuffer getTransferCommandBuffer();
    static VkQueue getGraphicsQueue();
    static VkQueue getPresentQueue();
    static VkQueue getTransferQueue();
    static VkCommandPool getGraphicsCommandPool();
    static VkCommandPool getTransferCommandPool();
    static QueueFamilyIndices getQueueFamilyIndices();
    static VkSurfaceKHR getSurface();
    static VkSampleCountFlagBits getMsaaSamples();
    static VkPhysicalDevice getPhysicalDevice();

private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;

    std::string windowTitle = "Vulkan Test Game Engine - FPS: ";
    float frameCount = 0;

    float xVel, yVel, zVel;
    uint32_t mipLevels;
    uint32_t WIDTH, HEIGHT;
    std::vector<VkCommandBuffer> drawCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores, renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences, imagesInFlight;
    static Swapchain *swapchain;
    static Camera *cam;
    static SwapchainSupportDetails swapchainSupport;
    static Framebuffer *framebuffer;
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

    void createRenderPass();

    void createPipeline();

    void createCommandPool();

    void allocateDrawCommandBuffers();

    void populateDrawCommandBuffer(size_t index);

    void createSyncObjects();

    bool waitForFence(uint32_t &imageIndex);

    void submitQueue(VkSemaphore signalSemaphores[], uint32_t imageIndex);

    void presentQueueToScreen(uint32_t &imageIndex, VkSemaphore signalSemaphores[]);

    void handleKeyPress(GLFWwindow *window);

    static void handleMouse(GLFWwindow *window, double x_pos, double y_pos);

    static void updateCamera();

    void updateUniformBuffer(uint32_t currentImage, Model *m);

    void recreateSwapchain();

    void cleanupSwapchain();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice testDevice);

    glm::quat angleBetweenVectors(glm::vec3 start, glm::vec3 end);
};

#endif