#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include<type_traits>
#include <glm/gtx/hash.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // Necessary for std::min/std::max
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <array>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <chrono>
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif





class TestEngine{
    public:
        void run(){
            initWindow();
            initVulkan();
            loop();
            cleanup();

        }

    private: 

        

        void initVulkan(){
            createInstance();//creates a vulkan instance
            setupDebugMessenger();
            createSurface();// creates glfw window using window params setup in initWindow()
            pickPhysicalDevice();//picks gpu
            createLogicalDevice();
            createSwapChain();
            createImageViews();
            createRenderPass();
            createDescriptorSetLayout();
            createGraphicsPipeline();
            createCommandPool();
            createColorResources();
            createDepthResources();
            createFramebuffers();
            createTextureImage();//uses command buffer
            createTextureImageView();
            createTextureSampler();
            loadModel();
            createVertexBuffer();//uses command buffer
            createIndexBuffer();//uses command buffer
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
            createCommandBuffers();
            createSyncObjects();
        }

        void loop(){
            auto start = std::chrono::high_resolution_clock::now();
            while (!glfwWindowShouldClose(window)) {
                    glfwPollEvents();
                    drawFrame();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    duration = duration/1000.0f;
                    frameCount ++;
                    if(duration >= 1 ){
                        fps = frameCount/duration;
                        glfwSetWindowTitle(window, (windowTitle + std::to_string(fps)).c_str());
                        start = std::chrono::high_resolution_clock::now();
                        frameCount = 0;
                    }
                    
            }
            vkDeviceWaitIdle(device);
        }       

        
        

        

        

        




        VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool){
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = pool;
            allocInfo.commandBufferCount = 1;
            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            return commandBuffer;
        }

        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue){
            vkEndCommandBuffer(commandBuffer);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);
            vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
        }   

    

    

    
};


int main() {
    TestEngine app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}