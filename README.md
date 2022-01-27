# Vulkan Test Game Engine
A 3D rendering program made with vulkan. 

# Before Building Code
> This code has a few dependencies that you need to obtain before building. (Makefile might need to be edited depending on the location of the dependency)
- Vulkan SDK
- Assimp
- STB
- GLFW
- GLM

## For Windows Users (using visual studio code 2022 or any other version):
1. Download the glfw binaries. glm, and stb.
2. Install the Vulkan SDK and assimp.
3. Copy the assimp-vc140-mt.dll from the bin/x64 folder in the assimp folder on your system, to the src folder. 
4. In Visual Studio under the Project tab click Properties (Make sure the Project name `VulkanTest` is selected under Solution Explorer first).
5. Make sure `Additional Include Directories` under `C/C++ -> General` includes the paths to the include folders for VulkanSDK, glfw, Assimp, and the vulkan-test-game-engine include folder. It should also point to the folder for stb and glm.
6. `Additional Library Directories` under `Linker -> General` should include the paths for the VulkanSDK Lib folder, the glfw lib folder for the corresponding version of visual studio, and the assimp lib folder.
7. `Additional Dependencies` under `Linker -> Input` should have vulkan-1.lib, glfw3.lib, and assimp-vc140-mt.lib. If they are not there then add them.
8. The `C++ Language Standard` under `Configuration Properties -> General` should be set to C++ 17. 

## For Linux Users:
1. Install vulkan, glfw, glm, and assimp through the package manager. 
2. Create a folder called libraries in the same directory as the vulkan-test-game-engine repo.
3. Download STB and place the stb folder in the libraries folder.

### Ubuntu commands to instal vulkan, glfw, glm, and assimp as an example:
- For glfw: `sudo apt-get install libglfw3-dev`
- For Vulkan: `sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools`
- For GLM: `sudo apt-get install libglm-dev`
- For Assimp: `sudo apt-get install libassimp-dev`

# Building Code

## For Windows Users
1. Click `Build Solution` under `Build`

## For Linux Users
1. A makefile is provided in the src file so to build in debug mode type `make debug` in the terminal otherwise type `make`.

# Compiling Shader Code
## A shell script is provided for compiling shader code. Just run it and it should compile any changes you made.

