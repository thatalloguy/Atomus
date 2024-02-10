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

#include <VkBootstrap.h>

#include <chrono>
#include <thread>

class VulkanEngine {

    public:
        // engine own variables
        bool _isInitialized{ false };
        int _frameNumber{0};
        bool _stopRendering{ false };

        VkExtent2D _windowExtent{1280, 720};
        GLFWwindow* _window{ nullptr };

        //Vulkan types
        VkInstance _instance;
        VkDebugUtilsMessengerEXT  _debugMessenger;
        VkPhysicalDevice _chosenGPU;
        VkDevice _device;
        VkSurfaceKHR _surface;

        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        VkExtent2D _swapchainExtent;


        // -_-
        static VulkanEngine& Get();


        // Life-time functions

        void Init();
        void CleanUp();
        void Draw();
        void Run();

    private:
        void initVulkan();
        void initSwapchain();
        void initCommands();
        void initSyncStructures();

        // swapchain lifetime functions
        void createSwapchain(uint32_t width, uint32_t height);
        void destroySwapchain();
};


#endif
