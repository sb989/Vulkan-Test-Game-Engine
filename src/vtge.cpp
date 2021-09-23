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
            Graphics *g = new Graphics(800,600,"");
            loop(*g);
        }

    private: 
        float                           fps = 0.0f;
        std::string windowTitle = "VTGE | FPS:";
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
            std::cout<<"out of whikle"<<std::endl;
            vkDeviceWaitIdle(device);
        }           
};

int main() {
    TestEngine app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cout<<"something broke"<<std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}