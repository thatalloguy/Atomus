//
// Created by allos on 2/10/2024.
//
#pragma once

// Includes and stuff
#include "VkEngine.h"

// Lets hope including sdl dont break me entire project :|
// SPOILER it did :( so ima use this hacky solution


// Some other variables here :)
constexpr bool bUseValidationLayers = false;

VulkanEngine* loadedEngine = nullptr;

VulkanEngine& VulkanEngine::Get() { return *loadedEngine; };


// MAIN STUFF (init function)

void VulkanEngine::Init() {

    // We cant load this shit twice :(

    assert(loadedEngine == nullptr);
    loadedEngine = this;

    if (!glfwInit()) {
        spdlog::error("Couldn't initialize GLFW\n");
    }


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW not to create an OpenGL context
    _window = glfwCreateWindow(800, 600, "Vulkan Engine :)", nullptr, nullptr);

    if (!_window) {
        spdlog::error("Couldn't create GLFW window\n");
        glfwTerminate();
    } else {
        spdlog::info("Created Window Successfully!");
    }

    // Loaded and ready to GO!!!!
    _isInitialized = true;

}

void VulkanEngine::CleanUp()
{
    if (_isInitialized) {
        spdlog::info("Destroying current loaded engine");
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    // clear engine pointer
    loadedEngine = nullptr;
}

void VulkanEngine::Draw()
{
    // As empty as my brain :)
}

void VulkanEngine::Run()
{

    spdlog::info("Calling main loop");
        // Handle events on queue
        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();

            if (glfwGetWindowAttrib(_window, GLFW_ICONIFIED)) {
                _stopRendering = true;
            }
            if (!glfwGetWindowAttrib(_window, GLFW_ICONIFIED)) {
                _stopRendering = false;
            }

            // do not draw if we are minimized
            if (_stopRendering) {
                // throttle the speed to avoid the endless spinning
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            Draw();
        }
}
