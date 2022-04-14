#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_texture.hpp"
#include "vtge_descriptor.hpp"
#include "vtge_light.hpp"
#include "vtge_camera.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_graphics.hpp"
#include "glm/gtx/string_cast.hpp"

#include <iostream>
#include <cstring>
static std::vector<Object *> objectList{};
static int g_index = 0;
Object::Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath, std::string specularMapPath, glm::vec4 color, std::string colorName)
{
    this->m = new Model(modelPath, imageCount, diffuseMapPath, specularMapPath, color, colorName);
    this->imageCount = imageCount;
    this->objIndex = g_index;
    g_index++;
    objectList.push_back(this);
    isVisible = true;
}

void Object::createObject(std::string modelPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, std::string diffuseMapPath, std::string specularMapPath)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Object *obj = new Object(modelPath, imgCount, diffuseMapPath, specularMapPath);
    setObjectTransform(obj, translate, scale, rotate);
}

void Object::createObject(std::string modelPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, glm::vec4 color, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Object *obj = new Object(modelPath, imgCount, "", "", color, colorName);
    setObjectTransform(obj, translate, scale, rotate);
}

std::vector<Object *> Object::getObjectList()
{
    return objectList;
}

Model *Object::getModel()
{
    return m;
}

void Object::setObjectTransform(Object *obj, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate)
{
    obj->getModel()->moveModel(translate);
    obj->getModel()->rotateModel(rotate);
    obj->getModel()->scaleModel(scale);
}

Object::~Object()
{
    delete m;
}

void Object::drawAllObjects(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index)
{
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < objectList.size(); i++)
    {
        Model *m = objectList[i]->getModel();
        glm::mat4 modelMat = m->getModelMat();

        if (i == 0)
        {
            Camera *cam = Graphics::getCamera();

            glm::mat4 modelView = cam->getViewMat() * modelMat;

            // std::cout << glm::to_string(modelView) << "used in normal draw object 0" << std::endl;
        }

        if (objectList[i]->getIsVisible())
            objectList[i]->drawObject(commandBuffer, pipeline->getPipelineLayout(), index);
    }
}

void Object::drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index)
{
    // draw object model and anything else thats needs to be drawn with the object
    std::vector<VkDescriptorSet> combinedDescriptorSets = {(*Light::getDescriptorSets())[index], (*Light::getShadowMapDescriptorSet())[index]};
    m->drawModel(combinedDescriptorSets, commandBuffer, pipelineLayout, index);
}

void Object::updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    for (int i = 0; i < objectList.size(); i++)
    {
        objectList[i]->updateObject(currentImage, projection, view);
    }
}

void Object::updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    m->updateModelMat(currentImage, projection, view);
}

void Object::recreateAllObjects(uint32_t imageCount)
{
    std::cout << objectList.size() << std::endl;
    for (int i = 0; i < objectList.size(); i++)
    {
        objectList[i]->m->recreateModel(imageCount);
    }
}

void Object::cleanupMemory()
{
    m->cleanupMemory();
}

void Object::destroyAllObjects()
{
    for (int i = 0; i < objectList.size(); i++)
    {
        delete objectList[i];
    }
    objectList.clear();
}

void Object::cleanupAllMemory()
{
    for (int i = 0; i < objectList.size(); i++)
    {
        objectList[i]->cleanupMemory();
    }
}
