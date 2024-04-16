//
// Created by allo on 2/10/2024.

/*
 * Purpose of this file (for future reference):
 * -This will be the main class for the (render) engine, and where most of the code will go.
 */



#pragma once

#include "VkInitializers.h"
#include "VkTypes.h"
#include "VkDescriptors.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct FrameData {

    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    VkSemaphore _swapchainSemaphore, _renderSemaphore;
    VkFence _renderFence;

    DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;


class VulkanEngine {

    public:


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

        // swapchain
        VkSwapchainKHR _swapchain;
        VkFormat _swapchainImageFormat;

        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        VkExtent2D _swapchainExtent;

        // Command stuff idk

        FrameData _frames[FRAME_OVERLAP];

        FrameData& getCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

        VkQueue _graphicsQueue;
        uint32_t _graphicsQueueFamily;


        DeletionQueue _mainDeletionQueue;
        VmaAllocator _allocator;

        // draw resources
        AllocatedImage _drawImage;
        VkExtent2D _drawExtent;

        //Descriptor stuff ;-;
        DescriptorAllocator globalDescriptorAllocator;

        VkDescriptorSet _drawImageDescriptors;
        VkDescriptorSetLayout _drawImageDescriptorLayout;

        // Life-time functions
        static VulkanEngine& Get();

        void Init();
        void CleanUp();
        void Draw();
        void Run();

    private:
        void initVulkan();
        void initSwapchain();
        void initCommands();
        void initSyncStructures();

        void drawBackground(VkCommandBuffer cmd);

        // swapchain lifetime functions
        void createSwapchain(uint32_t width, uint32_t height);
        void destroySwapchain();

        void initDescriptors();
};


