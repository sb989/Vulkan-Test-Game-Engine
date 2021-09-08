#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include "vtge_graphics.hpp"



#ifdef NDEBUG
    bool enableValidationLayers = false;
#else
    bool enableValidationLayers = true;
#endif

extern VkDevice device;

class TestEngine{
    public:
        void run(){
            Graphics g(640,320,enableValidationLayers,"");
            loop(g);
        }

    private: 
        float                           fps = 0.0f;
        std::string windowTitle = "VTGE | FPS:";
/*
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
*/
        void loop(Graphics g){
            auto start = std::chrono::high_resolution_clock::now();
            int frameCount = 0;
            while (!glfwWindowShouldClose(g.window)) {
                    glfwPollEvents();
                    g.drawFrame();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    duration = duration/1000.0f;
                    frameCount ++;
                    if(duration >= 1 ){
                        fps = frameCount/duration;
                        glfwSetWindowTitle(g.window, (windowTitle + std::to_string(fps)).c_str());
                        start = std::chrono::high_resolution_clock::now();
                        frameCount = 0;
                    }
            }
            vkDeviceWaitIdle(device);
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