#include <iostream>
#include "Renderer/VkEngine.h"


int main() {
    VulkanEngine engine;

    engine.Init();

    engine.Run();

    engine.CleanUp();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
