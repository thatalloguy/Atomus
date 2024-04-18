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
#include "VkLoader.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct FrameData {

    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    VkSemaphore _swapchainSemaphore, _renderSemaphore;
    VkFence _renderFence;

    DeletionQueue _deletionQueue;
    DescriptorAllocatorGrowable _frameDescriptors;
};


struct ComputePushConstants {
    glm::vec4 data1;
    glm::vec4 data2;
    glm::vec4 data3;
    glm::vec4 data4;
};

struct ComputeEffect {
    const char* name;

    VkPipeline pipeline;
    VkPipelineLayout layout;

    ComputePushConstants data;
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
        AllocatedImage _depthImage;
        VkExtent2D _drawExtent;

        //Descriptor stuff ;-;
        DescriptorAllocator globalDescriptorAllocator;

        VkDescriptorSet _drawImageDescriptors;
        VkDescriptorSetLayout _drawImageDescriptorLayout;

        //Pipeline :)
        VkPipeline _gradientPipeline;
        VkPipelineLayout _gradientPipelineLayout;


        //ImGui impl
        VkFence _immFence;
        VkCommandBuffer _immCommandBuffer;
        VkCommandPool _immCommandPool;

        void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void  destroyBuffer(const AllocatedBuffer& buffer);

        GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

        AllocatedImage createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
        AllocatedImage createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

        void destroyImage(const AllocatedImage& img);

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
        void resizeSwapchain();
        void destroySwapchain();

        void initDescriptors();

        void initPipelines();
        void initBackgroundPipelines();

        void initImGui();
        void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);

        std::vector<ComputeEffect> backgroundEffects;
        int currentBackgroundEffect{0};

        VkPipelineLayout _trianglePipelineLayout;
        VkPipeline _trianglePipeline;

        void initTrianglePipeline();
        void drawGeometry(VkCommandBuffer cmd);


        VkPipelineLayout _meshPipelineLayout;
        VkPipeline _meshPipeline;

        GPUMeshBuffers rectangle;

        void initMeshPipeline();

        bool resizeRequested = false;

        // MORE TEMP RAHHH
        AllocatedImage _whiteImage;
        AllocatedImage _blackImage;
        AllocatedImage _greyImage;
        AllocatedImage _errorImage;

        VkSampler _defaultSamplerLinear;
        VkSampler _defaultSamplerNearest;

        std::vector<std::shared_ptr<MeshAsset>> testMeshes;
        glm::vec3 monkeyPos{ 0, 0, 0};

        //scene
        GPUSceneData sceneData;

        VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;

        void initDefaultData();
};


