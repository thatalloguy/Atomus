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

### ScreenShot

![image](https://github.com/thatalloguy/Atomus/assets/51132972/5b95f56a-56e6-458d-ac4c-03b183c7b398)


### How to use

```c++
////TODO: better docs

#include "Renderer/VkEngine.h"

int main() {
    VulkanEngine engine;

    engine.Init();

    engine.Run();

    engine.CleanUp();

    return 0;
}
```
