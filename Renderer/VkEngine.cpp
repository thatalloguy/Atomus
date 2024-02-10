//
// Created by allos on 2/10/2024.
//
#pragma once

// Includes and stuff
#include "VkEngine.h"

constexpr bool bUseValidationLayers = false;

VulkanEngine* loadedEngine = nullptr;

VulkanEngine& VulkanEngine::Get() { return *loadedEngine; };



void VulkanEngine::Init() {


    assert(loadedEngine == nullptr);
    loadedEngine = this;

    if (!glfwInit()) {
        spdlog::error("Couldn't initialize GLFW\n");
    } else {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        _window = glfwCreateWindow(800, 600, "Vulkan Engine :)", nullptr, nullptr);

        if (!_window) {
            spdlog::error("Couldn't create GLFW window\n");
            glfwTerminate();
        } else {
            spdlog::info("Created Window Successfully!");
        }

        // Initialize vulkan

        spdlog::info("initializing Vulkan!");

        initVulkan();
        initSwapchain();
        initCommands();
        initSyncStructures();


        _isInitialized = true;
    }

}

void VulkanEngine::CleanUp()
{
    if (_isInitialized) {
        spdlog::info("Destroying current loaded engine");

        destroySwapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);

        vkDestroyInstance(_instance, nullptr);
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



void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder builder; // bob the builder :)

    auto inst_ret = builder.set_app_name("Atomus Application")
            .request_validation_layers(bUseValidationLayers)
            .use_default_debug_messenger()
            .require_api_version(1, 3, 0)
            .build(); // TODO replace later one with user one

    vkb::Instance vkb_inst = inst_ret.value();

    // now update our instance :)
    _instance = vkb_inst.instance;
    _debugMessenger = vkb_inst.debug_messenger;


    if (glfwCreateWindowSurface(_instance, _window, NULL, &_surface) != VK_SUCCESS) {
        spdlog::error("Could not Create window Surface !");
        return;
    }

    VkPhysicalDeviceVulkan13Features features{};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // select a GPU that can write to glfw and supports vulkan 1.3
    vkb::PhysicalDeviceSelector selector{ vkb_inst};
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(_surface)
            .select()
            .value();

    // Final Vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;


    // Use vkBootstrap to get a Graphics queue

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

}

void VulkanEngine::initSwapchain() {
    createSwapchain(_windowExtent.width, _windowExtent.height);
}

void VulkanEngine::initCommands() {

}

void VulkanEngine::initSyncStructures() {

}

void VulkanEngine::createSwapchain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU, _device, _surface };

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
            .set_desired_format(VkSurfaceFormatKHR{ _swapchainImageFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

    _swapchainExtent = vkbSwapchain.extent;

    // Store swapchain and it's related images
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanEngine::destroySwapchain() {
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    for (auto & _swapchainImageView : _swapchainImageViews) {
        vkDestroyImageView(_device, _swapchainImageView, nullptr);
    }
}
