#ifndef __VTGE_CAMERA_HPP__
#define __VTGE_CAMERA_HPP__
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera
{
public:
    Camera(float width, float heigth);
    void handleKeyPress(GLFWwindow *window);
    void handleMouse(GLFWwindow *window);
    void updateCamera();
    glm::mat4 getViewMat();
    glm::mat4 getProjectionMat();

private:
    float camXPos, camYPos, camZPos;
    float oldCamYaw, oldCamPitch, camYaw, camPitch;
    float cursorXPos, cursorYPos;
    glm::mat4 viewMat, projectionMat;
    glm::vec3 lookDir;
    glm::vec3 camPos;

    glm::mat4 makeRotationMatrix(glm::vec3 rightVec, glm::vec3 upVec, glm::vec3 forwardVec, glm::vec3 postion);
    glm::mat4 makeTranslationMatrix(glm::vec3 postion);
};

#endif