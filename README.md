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
- [ ] Push Constants and new shaders.
-
- [ ] Set up the render pipeline
- [ ] Mesh Buffers
- [ ] Mesh loading
- [ ] Blending
- [ ] Window Resizing
-
- [ ] Descriptors Abstractions
- [ ] Textures
- [ ] Engine Architecture
- [ ] Materials
- [ ] Meshes & camera
-
- [ ] Interactive Camera
- [ ] GLTF scene nodes.
- [ ] GLTF textures
- [ ] Faster draw
-

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
