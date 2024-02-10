//
// Created by allo on 2/10/2024.

/*
 * Purpose of this file (for future reference):
 * -This will be the main class for the (render) engine, and where most of the code will go.
 */



#ifndef ATOMUSVULKAN_VKENGINE_H
#define ATOMUSVULKAN_VKENGINE_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VkInitializers.h"
#include "VkTypes.h"

#include <chrono>
#include <thread>

class VulkanEngine {

    public:
        // engine own variables
        bool _isInitialized{ false };
        int _frameNumber{0};
        bool _stopRendering{ false };

        // Extentstional types and stuff
        VkExtent2D _windowExtent{1280, 720};
        GLFWwindow* _window{ nullptr };

        // -_-
        static VulkanEngine& Get();


        // Life-time functions

        void Init();

        void CleanUp();


        /// Draws the application
        void Draw();

        /// runs the main loop (NOT DRAWING)
        void Run();
};


#endif
