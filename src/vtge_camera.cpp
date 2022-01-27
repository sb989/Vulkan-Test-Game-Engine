#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "vtge_camera.hpp"
#include "vtge_graphics.hpp"
#include "vtge_swapchain.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
Camera::Camera(float width, float height)
{
    camXPos = camYPos = camZPos = 0.0f;
    cursorXPos = cursorYPos = 0.0f;
    camPos = glm::vec3(camXPos, camYPos, camZPos);
    lookDir = glm::vec3(0.0f, 0.0f, -1.0f);
    viewMat = glm::lookAt(glm::vec3(camXPos, camYPos, camZPos), lookDir + camPos, glm::vec3(0.0f, 1.0f, 0.0f));
    projectionMat = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 400.0f);
    camYaw = camPitch = oldCamPitch = oldCamYaw = 0.0f;
}

void Camera::handleKeyPress(GLFWwindow *window)
{
    int up_state = glfwGetKey(window, GLFW_KEY_W);
    int down_state = glfwGetKey(window, GLFW_KEY_S);
    int left_state = glfwGetKey(window, GLFW_KEY_A);
    int right_state = glfwGetKey(window, GLFW_KEY_D);
    int esc_state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    int enter_state = glfwGetKey(window, GLFW_KEY_ENTER);
    int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
    int shift_state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

    if (up_state == GLFW_PRESS || up_state == GLFW_REPEAT && down_state != GLFW_REPEAT)
    {
        camXPos -= .3f * lookDir.x;
        camZPos -= .3f * lookDir.z;
    }
    else if (down_state == GLFW_PRESS || down_state == GLFW_REPEAT && up_state != GLFW_REPEAT)
    {
        camXPos += .3f * lookDir.x;
        camZPos += .3f * lookDir.z;
    }

    if (left_state == GLFW_PRESS || left_state == GLFW_REPEAT && right_state != GLFW_REPEAT)
    {
        camXPos -= .3f * lookDir.z;
        camZPos += .3f * lookDir.x;
    }
    else if (right_state == GLFW_PRESS || right_state == GLFW_REPEAT && left_state != GLFW_REPEAT)
    {
        camXPos += .3f * lookDir.z;
        camZPos -= .3f * lookDir.x;
    }

    if (esc_state == GLFW_PRESS || esc_state == GLFW_REPEAT && enter_state != GLFW_REPEAT)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
    else if (enter_state == GLFW_PRESS || enter_state == GLFW_REPEAT && esc_state != GLFW_REPEAT)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        glfwSetCursorPos(window, 0, 0);
    }

    if (space_state == GLFW_PRESS || space_state == GLFW_REPEAT && shift_state != GLFW_REPEAT)
    {
        camYPos += .3f;
    }
    else if (shift_state == GLFW_PRESS || shift_state == GLFW_REPEAT && space_state != GLFW_REPEAT)
    {
        camYPos -= .3f;
    }
}

void Camera::handleMouse(GLFWwindow *window)
{
    double x_pos, y_pos;
    glfwGetCursorPos(window, &x_pos, &y_pos);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        float xdiff = x_pos - cursorXPos;
        float ydiff = y_pos - cursorYPos;
        camYaw += xdiff * .05;
        camPitch += ydiff * .05;
        cursorXPos = x_pos;
        cursorYPos = y_pos;
    }
}

void Camera::updateCamera()
{
    //i used quaterions becuz i was curious how they worked
    //probably could have done it with less code if i didnt create my own lookat matrix
    //or if i didnt use quaterions
    glm::quat rotateQuat;
    if (lookDir.y <= -0.9 && camPitch < oldCamPitch || lookDir.y >= 0.9 && camPitch > oldCamPitch)
    {
        camPitch = oldCamPitch;
    }
    glm::vec3 worldup = glm::vec3(0.0f, 1.0f, 0.0f);
    rotateQuat = glm::angleAxis(glm::radians(oldCamPitch - camPitch), glm::cross(worldup, lookDir));
    rotateQuat = glm::angleAxis(glm::radians(oldCamYaw - camYaw), worldup) * rotateQuat;

    camPos = glm::vec3(camXPos, camYPos, camZPos);
    lookDir = rotateQuat * lookDir * glm::inverse(rotateQuat);
    lookDir = glm::normalize(lookDir);

    glm::vec3 newCamRight = glm::normalize(glm::cross(worldup, lookDir));
    glm::vec3 newCamUp = glm::normalize(glm::cross(newCamRight, lookDir));

    viewMat = glm::mat4(
        glm::vec4(newCamRight.x, newCamUp.x, lookDir.x, 0),
        glm::vec4(newCamRight.y, newCamUp.y, lookDir.y, 0),
        glm::vec4(newCamRight.z, newCamUp.z, lookDir.z, 0),
        glm::vec4(-dot(newCamRight, camPos), -dot(newCamUp, camPos), -dot(lookDir, camPos), 1));
    oldCamPitch = camPitch;
    oldCamYaw = camYaw;
}

glm::mat4 Camera::getViewMat()
{
    return viewMat;
}

glm::mat4 Camera::getProjectionMat()
{
    return projectionMat;
}