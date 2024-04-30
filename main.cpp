#pragma once
#include "Renderer/VkEngine.h"

int main() {
    VulkanEngine engine;


    ///VulkanEngine doenst handle this anymore.
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window =  glfwCreateWindow(800, 600, "Atomus 0.0.1", nullptr, nullptr);

    engine.Init(window, true);

    //load test model
    std::string structurePath = {"..//..//Assets/structure_mat.glb"};
    auto structureFile = VkLoader::loadGltf(&engine, structurePath);

    assert(structureFile.has_value());

    engine.loadedScenes["structure"] = *structureFile;


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        engine.updateScene();

        engine.loadedScenes["structure"]->Draw(glm::mat4{1.f}, engine.mainDrawContext);

        engine.Run();
    }


    engine.CleanUp();

    return 0;
}
