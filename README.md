# Atomus
My personal vulkan renderer

## To do:
- [x] Initializing Vulkan.
- [ ] Shaders.
- [ ] Setup imgui.
- [ ] Add more todo's.

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
