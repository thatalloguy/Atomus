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
- [ ] GLTF textures
- [ ] Faster draw

##### Known bugs
-[ ] Perspective gets stretched when rotating
-[ ] LoadGLTF has vector out of sub range

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
