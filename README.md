# Atomus
My personal vulkan renderer

## To do:
- [x] Initializing Vulkan.
- [x] Executing Commands
- [x] Setting up commands
- [x] Rendering Loop
- [x] MainLoop
- [x] Improve render loop
-
- [x] Shaders.
- [x] Setup imgui.
- [x] Push Constants and new shaders.
-
- [x] Set up the render pipeline
- [x] Mesh Buffers
- [x] Mesh loading
- [x] Blending
- [x] Window Resizing
-
- [x] Descriptors Abstractions
- [x] Textures
- [x] Engine Architecture
- [x] Materials
- [x] Meshes & camera
-
- [x] Interactive Camera
- [x] GLTF scene nodes.
- [x] GLTF textures
- [x] Faster draw
-
- [x] Refactor

### ScreenShot

![image](https://github.com/thatalloguy/Atomus/assets/51132972/5b95f56a-56e6-458d-ac4c-03b183c7b398)


### How to use

```c++

#pragma once
#include "Renderer/VkEngine.h"

int main() {
VulkanEngine engine;


///VulkanEngine doenst handle this anymore. you gotta do window creation with GLFW!
glfwInit();
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
auto window =  glfwCreateWindow(800, 600, "Atomus 0.0.1", nullptr, nullptr);

// pass in a pointer GLFWwindow object and enable or disable a imgui debug menu (false by default);
engine.Init(window, true);

//loading a model (Currently supports GLTF and GLB, via the fastgltf lib)
std::string structurePath = {"..//..//Assets/structure_mat.glb"};
auto structureFile = VkLoader::loadGltf(&engine, structurePath);
// just a check, not necessary
assert(structureFile.has_value());
//Name can be anything.
engine.loadedScenes["structure"] = *structureFile;


while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    
    //First update the scene
    engine.updateScene();
    
    //Draw any objects after that
    engine.loadedScenes["structure"]->Draw(glm::mat4{1.f}, engine.mainDrawContext);
    
    //Then call the run function
    engine.Run();
}

/// Any objects in the loadedScenes structure gets handeld by the engine and doenst have to be manually to destroyed.
engine.CleanUp();

return 0;
}
```
