#ifndef __VTGE_HPP__
#define __VTGE_HPP__
#include <string>
class Graphics;
class TestEngine
{
public:
    void run();

private:
    float fps = 0.0f;
    std::string windowTitle = "VTGE | FPS:";
    const std::string VIKING_MODEL_PATH = "../models/viking_room.obj";
    const std::string VIKING_TEXTURE_PATH = "../textures/viking_room.png";
    const std::string BANANA_MODEL_PATH = "../models/ripe-banana.obj";
    const std::string BANANA_TEXTURE_PATH = "../textures/ripe-banana_u1_v1.png";
    void loop(Graphics *g);
};

#endif
