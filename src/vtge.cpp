#define GLFW_INCLUDE_VULKAN
#include "vtge.hpp"
#include "vtge_graphics.hpp"
#include "vtge_light.hpp"
#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

void TestEngine::run()
{
    Graphics *g = new Graphics(800, 600, "");
    loop(g);
}

void TestEngine::loop(Graphics *g)
{
    auto start = std::chrono::high_resolution_clock::now();

    int frameCount = 0;
    Graphics::beginGraphicsCommandBuffer();
    Graphics::beginTransferCommandBuffer();
    VkDevice device = Graphics::getDevice();
    Light::createDirectionalLight("../models/cube.obj", glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, -1, 0), glm::vec4(0, 0, 2, 1),
                                  glm::vec4(1), glm::vec4(.0, .0, .0, 1), glm::vec4(1), "white");

    Light::createPointLight("../models/cube.obj", glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(15, 5, 1, 1),
                            glm::vec4(1, 0, 0, 1), glm::vec4(.2, .2, .2, 1), glm::vec4(1), 1.0f, 0.14f, 0.07f, "red");
    Light::createSpotLight("../models/cube.obj", glm::vec3(1), glm::vec3(0), glm::vec3(0, -1, 0), glm::vec4(4, 2, 3, 1), glm::vec4(1, 1, 0, 1),
                           glm::vec4(1, 1, 0, 1), glm::vec4(1), 1.0f, 0.09f, 0.032f, glm::cos(glm::radians(12.5)), glm::cos(glm::radians(15.0f)), "yellow");
    std::cout << glm::cos(glm::radians(12.5)) << std::endl;
    Object::createObject("../models/cube.obj", glm::vec3(2, 14, 5), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), "../textures/crate_diffuse.png", "../textures/crate_specular.png");

    Object::createObject("../models/cube.obj", glm::vec3(2, 10, 5), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0, 200, 125, 255), "ugly");

    Object::createObject(BANANA_MODEL_PATH, glm::vec3(2, 0, 5), glm::vec3(0.05f), glm::vec3(0.0f, 90.0f, 0.0f), BANANA_TEXTURE_PATH);

    Object::createObject(VIKING_MODEL_PATH, glm::vec3(-4, 5, 5), glm::vec3(1.0f), glm::vec3(0.0f), VIKING_TEXTURE_PATH);

    Object::createObject("../models/porsche.gltf", glm::vec3(10, 10, 10), glm::vec3(1), glm::vec3(0, 0, 0));
    //Object::createObject("../models/square.gltf", glm::vec3(0, 0, 0), glm::vec3(1), glm::vec3(0));
    Object::createObject("../models/square.gltf", glm::vec3(0, -5, 0), glm::vec3(1), glm::vec3(0));
    Object::createObject("../models/sphere.gltf", glm::vec3(5, 0, 5), glm::vec3(1), glm::vec3(0));
    Graphics::endTransferCommandBuffer();
    Graphics::endGraphicsCommandBuffer();
    float spotlightPosChange = 0.3f;
    while (!glfwWindowShouldClose(g->window))
    {
        Light *spot = Light::getSpotLight(0);
        if (spot != nullptr)
        {
            float x_pos = spot->getModel()->getModelPos().x;
            std::cout << x_pos << std::endl;
            if (x_pos >= 5)
                spotlightPosChange = -0.3f;
            else if (x_pos <= -5)
                spotlightPosChange = 0.3f;
            spot->getModel()->moveModel(glm::vec3(spotlightPosChange, 0, 0));
        }
        glfwPollEvents();
        g->drawFrame();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        duration = duration / 1000.0f;
        frameCount++;
        if (duration >= 1)
        {
            fps = frameCount / duration;
            glfwSetWindowTitle(g->window, (windowTitle + std::to_string(fps)).c_str());
            start = std::chrono::high_resolution_clock::now();
            frameCount = 0;
        }
    }
    std::cout << "out of whikle" << std::endl;
    vkDeviceWaitIdle(device);
}

int main()
{
    TestEngine app;
    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cout << "something broke" << std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}