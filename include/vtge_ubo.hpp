#ifndef __VTGE_UBO_HPP__
#define __VTGE_UBO_HPP__
#include <glm/glm.hpp>

struct UniformBufferObject
{
    alignas(16) glm::mat4 normMatrix;
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

#endif
