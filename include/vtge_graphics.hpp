#ifndef __VTGE_GRAPHICS_HPP__
#define __VTGE_GRAPHICS_HPP__
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <GLFW/glfw3.h>
#include <vector>
#include "vtge_vertex.hpp"
#include <optional>
//#include "vtge_model.hpp"
#include "vtge_getter_and_checker_functions.hpp"
class Swapchain;
class Framebuffer;
class Pipeline;
class Object;
class Model;
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

struct PushConstants
{
    alignas(16) glm::mat4 normMatrix;
};
static float camXPos, camYPos, camZPos = 0;
static float oldCamYaw, oldCamPitch, camYaw, camPitch = 0;
static float cursorXPos, cursorYPos = 0;
static glm::mat4 viewMat, projectionMat = glm::mat4(0);
static glm::vec3 lookDir = glm::vec3(0);
static glm::vec3 camPos = glm::vec3(0);

class Graphics
{
public:
    Graphics(uint32_t width, uint32_t height, std::string windowTitle);

    ~Graphics();

    void drawFrame();

    bool framebufferResized = false;
    GLFWwindow *window;

private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;
    const std::string VIKING_MODEL_PATH = "../models/viking_room.obj";
    const std::string VIKING_TEXTURE_PATH = "../textures/viking_room.png";
    const std::string BANANA_MODEL_PATH = "../models/ripe-banana.obj";
    const std::string BANANA_TEXTURE_PATH = "../textures/ripe-banana_u1_v1.png";
    std::string windowTitle = "Vulkan Test Game Engine - FPS: ";
    float frameCount = 0;

    float xVel, yVel, zVel;
    uint32_t mipLevels;
    uint32_t WIDTH, HEIGHT;
    std::vector<VkCommandBuffer> drawCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores, renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences, imagesInFlight;
    //std::vector<Model*>             lightList; //modelList
    Swapchain *swapchain;
    SwapchainSupportDetails swapchainSupport;
    Framebuffer *framebuffer;
    VkInstance instance;
    VkRenderPass renderPass;
    Pipeline *graphicsPipeline, *lightPipeline;
    //VkDescriptorPool                descriptorPool;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

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

    void createObject(std::string modelPath, std::string diffuseMapPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, std::string specularMapPath = "");
    void createObject(std::string modelPath, glm::vec4 color, std::string colorName, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);
    void createObject(std::string modelPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);

    void setObjectTransform(Object *obj, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);

    void createPointLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 lightPos,
                          glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic, std::string color);

    void createDirectionalLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 direction, glm::vec4 lightPos,
                                glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string color);

    void createSpotLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 direction, glm::vec4 lightPos,
                         glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic,
                         float cutOff, float outerCutOff, std::string color);

    void recreateSwapchain();

    void cleanupSwapchain();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice testDevice);

    glm::quat angleBetweenVectors(glm::vec3 start, glm::vec3 end);
};

#endif