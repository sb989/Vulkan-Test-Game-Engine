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
class Pipeline;
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

struct PushConstants{
  //alignas(16)  glm::vec3 camPos;
  alignas(16) glm::mat4 normMatrix;
};

class Graphics{
    public:
        /**
         * @brief constructor for Graphics class
         * @param width starting width of the window created
         * @param height starting height of the window created
         * @param windowTitle title on the window created
         */
        Graphics(uint32_t width, uint32_t height, std::string windowTitle);
        
        /**
         * @brief deconstructor for Graphics class
         */
        ~Graphics();

        /**
         * @brief draws next frame that is ready from the swapchain
         */
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
        float                           oldCamYaw, oldCamPitch, camYaw, camPitch;
        double                          cursorXPos, cursorYPos;
        float                           xVel, yVel, zVel;
        uint32_t                        mipLevels;
        uint32_t                        WIDTH, HEIGHT;
        std::vector<VkCommandBuffer>    drawCommandBuffers;
        std::vector<VkSemaphore>        imageAvailableSemaphores, renderFinishedSemaphores;
        std::vector<VkFence>            inFlightFences, imagesInFlight;
        //std::vector<Model*>             lightList; //modelList
        Swapchain                       *swapchain;
        SwapchainSupportDetails         swapchainSupport;
        Framebuffer                     *framebuffer;
        VkInstance                      instance;
        VkRenderPass                    renderPass;
        Pipeline                      *graphicsPipeline, *lightPipeline;
        //VkDescriptorPool                descriptorPool;        
        VkDebugUtilsMessengerEXT        debugMessenger;
        VkSurfaceKHR                    surface;
        glm::mat4                       viewMat, projectionMat;
        glm::vec3                       lookDir;
        glm::vec3                       camPos;
        /**
         * @brief sets up and creates glfw window
         */
        void setUpWindow();

        /**
         * @brief handles everything that must be done to start drawing frames
         */
        void setUpGraphics();

        /**
         * @brief creates the vulkan instance that will be used
         */
        void createInstance();

        /**
         * @brief creates the surface on the glfw window that will be drawn on
         */
        void createSurface();

        /**
         * @brief picks the physical device(dedicated or integrated gpu) that will be used for rendering
         */
        void pickPhysicalDevice();

        /**
         * @brief creates the logical device(a device that sepcifies what features of the physical device will be used)
         */
        void createLogicalDevice();

        /**
         * @brief creates the renderpass (renderpasses decribe what images will be used, and how they will be used)
         */
        void createRenderPass();

        /**
         * @brief creates descriptor set layout using descriptorsetlayout bindings
         */
        //void createDescriptorSetLayout();

        /**
         * @brief creates pipelines
         */
        void createPipeline();

        
        /**
         * @brief creates a command pool
         */
        void createCommandPool();

        /**
         * @brief allocates a draw command buffers for every framebuffer
         */
        void allocateDrawCommandBuffers();

        /**
         * @brief populates a command buffer for each framebuffer that holds draw commands
         */
        void populateDrawCommandBuffer(size_t index);

        /**
         * @brief creates sync objects
         */
        void createSyncObjects();

        /**
         * @brief waits for the fence before
         * @return if it returns true it means what you wait for is available
         * if it returns false it means somethings needs to be recreated and you need to skip this render pass
         * 
         */
        bool waitForFence(uint32_t &imageIndex);

        void submitQueue(VkSemaphore signalSemaphores[], uint32_t imageIndex);

        void presentQueueToScreen(uint32_t &imageIndex, VkSemaphore signalSemaphores []);

        /**
         * @brief handles key presses
         * @param window the glfw window currently being used
         */
        void handleKeyPress(GLFWwindow* window);

        /**
         * @brief handlesMouseMovements
         * @param window the glfw window currenty being used
         */
        void handleMouse(GLFWwindow* window);

        /**
         * @brief updates camera position and orientation
         */
        void updateCamera();

        /**
         * @brief updates uniform buffer for a given model
         * @param currentImage the current image being drawn
         * @param m the model that needs it uniform buffer updated
         */
        void updateUniformBuffer(uint32_t currentImage, Model *m);

        /**
         * @brief creates a model object and adds it to the list of models
         * @param modelPath the path to the model
         * @param texturePath the path to the models texture
         * @param translate a glm::vec3 that translates the model a distance in a direction
         * @param scale a glm::vec3 that scales the model
         * @param rotate a glm::vec3 that rotates the model
         */
        void createObject(std::string modelPath, std::string texturePath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);


        /**
         * @brief creates a model object and adds it to the list of models
         * @param modelPath the path to the model
         * @param translate a glm::vec3 that translates the model a distance in a direction
         * @param scale a glm::vec3 that scales the model
         * @param rotate a glm::vec3 that rotates the model
         */
        void createLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 lightColor, glm::vec3 lightPos);


        /**
         * @brief destroys and recreates the swapchain; it is called whenever the swapchain becomes outdated
         */
        void recreateSwapchain();

        /**
         * @brief destroys anything related to the swapchain
         */
        void cleanupSwapchain();

        /**
         * @brief ends and submits commands in a commandbuffer to a queue
         * @param commandBuffer the command buffer whose commands will be submitted
         * @param pool the command pool that the command buffer belongs to; the command buffer is freed and returned to this pool
         * @param queue the queue that the commands in the command buffer are submitted to
         */
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);

        /**
         * @brief callback function that is called when the framebuffer is resized
         * @param window the window whose framebuffer is resized
         * @param width the width of the window
         * @param height the height of the window
         */
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

        /**
         * @brief finds the queue family index of the present, transfer, and graphics queue
         * @param device the physical device to find queue families for
         * @return returns a QueueFamilyIndices struct that contains the index of each queue family
         */
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        
        /**
         * @brief allocates a command buffer from the given command pool and allows it to start storing commands
         * @param the command pool to allocate the command buffer from
         * @return returns a command buffer
         */
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);

        /**
         * @brief fetches the swapchain support details for a given physicalDevice
         * @param testDevice the physical device to fetch swapchain support information about
         * @return returns a SwapchainSupportDetails object that constains support details about a given physical device
         */
        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice testDevice);

        /**
         * @brief finds the angle between two angles and returns a quaternion
         * so that quaternion q times vector start equals the vector end
         * q * start = end
         * @param start vector for the first anle
         * @param end vector for the second angle
         * @return returns a glm::quat (a quaternion)
         */
        glm::quat angleBetweenVectors(glm::vec3 start, glm::vec3 end);
};

#endif