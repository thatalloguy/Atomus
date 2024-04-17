//
// Created by allos on 2/10/2024.
//
#pragma once

// Includes and stuff
#include "VkEngine.h"
#include "VkImages.h"
#include "VkPipelines.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>


// Imgui includes
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

constexpr bool bUseValidationLayers = true;

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

        initDescriptors();

        initPipelines();
        initImGui();

        initDefaultData();
        _isInitialized = true;
    }

}

void VulkanEngine::initDescriptors() {
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
    };
    // 10 sets with 1 image each
    globalDescriptorAllocator.initPool(_device, 10, sizes);

    {
        ///TODO correctly destroy this obj.
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        _drawImageDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    //Allocator a descriptor set for the draw image
    _drawImageDescriptors = globalDescriptorAllocator.allocate(_device, _drawImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = _drawImage.imageView;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;

    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = _drawImageDescriptors;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(_device, 1, &drawImageWrite, 0, nullptr);

    _mainDeletionQueue.pushFunction([&]() {
        vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
    });
}


void VulkanEngine::CleanUp()
{
    if (_isInitialized) {
        spdlog::info("Destroying current loaded engine");
        vkDeviceWaitIdle(_device);


        _mainDeletionQueue.flush();

        for (int i=0; i < FRAME_OVERLAP; i++) {
            vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

            // destroy sync objects
            vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
            vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
            vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
        }


        destroySwapchain();

        globalDescriptorAllocator.destroyPool(_device);

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);

        vkDestroyInstance(_instance, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
        spdlog::info("Successfully destroyed current loaded engine");
    }

    // clear engine pointer
    loadedEngine = nullptr;
}

void VulkanEngine::Draw()
{
    // wait until the gpu has finished rendering the last frame
    VK_CHECK(vkWaitForFences(_device, 1, &getCurrentFrame()._renderFence, true, 1000000000));

    //Flush the current Frame deletion Que
    getCurrentFrame()._deletionQueue.flush();

    VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));

    // get the image from the swapchain
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, getCurrentFrame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

    VkCommandBuffer cmd = getCurrentFrame()._mainCommandBuffer;

    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    _drawExtent.width = _drawImage.imageExtent.width;
    _drawExtent.height = _drawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // transform our main image into a general one (for writing).
    // overwite it all since we dont care what the old layout was :shrug:
    VkUtil::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    drawBackground(cmd);

    VkUtil::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


    drawGeometry(cmd);

    //Transition the draw image (and swapchain image) into the correct transfer layours
    VkUtil::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkUtil::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkUtil::copyImageToImage(cmd, _drawImage.image, _swapchainImages[swapchainImageIndex],  _drawExtent, _swapchainExtent);

    //Set the swapchain layout for imgui stuff :)
    VkUtil::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //draw imgui into the swapchain img
    drawImgui(cmd, _swapchainImageViews[swapchainImageIndex]);

    // set the swapchain image layout to present
    VkUtil::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    //prepare the submit to queue
    // wait on _presentSemaphore. and _renderSemaphore

    VkCommandBufferSubmitInfo cmdInfo = VkInit::commandBufferSubmitInfo(cmd);

    VkSemaphoreSubmitInfo waitInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, getCurrentFrame()._swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);

    VkSubmitInfo2 submit = VkInit::submitInfo(&cmdInfo, &signalInfo, &waitInfo);

    // submit the cmdbuffer to the queue and execute it

    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, getCurrentFrame()._renderFence));


    // prepare present
    // this will put the image to the visible window

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    _frameNumber++;
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

            // Imgui
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();


            if (ImGui::Begin("Background")) {
                ComputeEffect& selected = backgroundEffects[currentBackgroundEffect];

                ImGui::Text("Selected Effect: %s", selected.name);

                ImGui::SliderInt("Effect Index", &currentBackgroundEffect, 0, backgroundEffects.size() - 1);

                ImGui::InputFloat4("data1", (float*)& selected.data.data1);
                ImGui::InputFloat4("data2", (float*)& selected.data.data2);
                ImGui::InputFloat4("data3", (float*)& selected.data.data3);
                ImGui::InputFloat4("data4", (float*)& selected.data.data4);

                ImGui::End();
            }

            ImGui::Render();

            Draw();
        }
}



void VulkanEngine::drawBackground(VkCommandBuffer cmd) {

    ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];

    // bind the background compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

    // bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_drawImageDescriptors, 0, nullptr);

    vkCmdPushConstants(cmd, _gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
    // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);
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

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    _mainDeletionQueue.pushFunction([&]() {
        vmaDestroyAllocator(_allocator);
    });



}

void VulkanEngine::initSwapchain() {
    createSwapchain(_windowExtent.width, _windowExtent.height);

    // draw image size will match the window :)

    VkExtent3D drawImageExtent = {
            _windowExtent.width,
            _windowExtent.height,
            1
    };

    // draw format is hardcoded to 32 bit float
    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    _drawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags  drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimgInfo = VkInit::imageCreateInfo(_drawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rimgAllocInfo = {};
    rimgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(_allocator, &rimgInfo, &rimgAllocInfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    VkImageViewCreateInfo rviewInfo = VkInit::imageViewCreateInfo(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(_device, &rviewInfo, nullptr, &_drawImage.imageView));

    _mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(_device, _drawImage.imageView, nullptr);
        vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
    });

}

void VulkanEngine::initCommands() {

    // create a command pool for commands sumbitted by the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = VkInit::commandPoolCreateInfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i=0; i < FRAME_OVERLAP; i++) {

        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

        // Allocate the default command buffer for rendering
        VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(_frames[i]._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
    }


    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_immCommandPool));

    //alloc the cmd buf for immediate submits

    VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(_immCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));

    _mainDeletionQueue.pushFunction([=]() {
        vkDestroyCommandPool(_device, _immCommandPool, nullptr);
    });

}

void VulkanEngine::initSyncStructures() {

    // 1 fence to control when the gpu finishes rendering the frame
    // and 2 semaphores to sync rendering with me swapchain :-)
    //

    VkFenceCreateInfo fenceCreateInfo = VkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VkInit::semaphoreCreateInfo();

    for (int i=0; i <FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
    }


    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));
    _mainDeletionQueue.pushFunction([=]() { vkDestroyFence(_device, _immFence, nullptr); });
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

void VulkanEngine::initPipelines() {
    initBackgroundPipelines();
    initTrianglePipeline();
    initMeshPipeline();
}

void VulkanEngine::initBackgroundPipelines() {

    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(_device, &computeLayout, nullptr, &_gradientPipelineLayout));

    //layout Code
    VkShaderModule gradientShader;
    if (!VkUtil::loadShaderModule("gradient.comp.spv", _device, &gradientShader)) {
        spdlog::error("[ENGINE] Error with Loading shader");
    }

    VkShaderModule skyShader;
    if (!VkUtil::loadShaderModule("sky.comp.spv", _device, &skyShader)) {
        spdlog::error("[ENGINE] Error with loading Sky shader!");
    }

    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = gradientShader;
    stageinfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = _gradientPipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;

    ComputeEffect gradient{};
    gradient.layout = _gradientPipelineLayout;
    gradient.name = "gradient";
    gradient.data = {};

//default colors
    gradient.data.data1 = glm::vec4(1, 0, 0, 1);
    gradient.data.data2 = glm::vec4(0, 0, 1, 1);

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline));

//change the shader module only to create the sky shader
    computePipelineCreateInfo.stage.module = skyShader;

    ComputeEffect sky{};
    sky.layout = _gradientPipelineLayout;
    sky.name = "sky";
    sky.data = {};


    sky.data.data1 = glm::vec4(0.1, 0.2, 0.4 ,0.97);

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline));

//add the 2 background effects into the array
    backgroundEffects.push_back(gradient);
    backgroundEffects.push_back(sky);

//destroy structures properly
    vkDestroyShaderModule(_device, gradientShader, nullptr);
    vkDestroyShaderModule(_device, skyShader, nullptr);
    _mainDeletionQueue.pushFunction([&]() {
        vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);

        for (auto effects : backgroundEffects) {
            vkDestroyPipeline(_device, effects.pipeline, nullptr);
        }
    });

}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {

    VK_CHECK(vkResetFences(_device, 1, &_immFence));
    VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));


    VkCommandBuffer cmd = _immCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = VkInit::commandBufferSubmitInfo(cmd);
    VkSubmitInfo2 submit = VkInit::submitInfo(&cmdInfo, nullptr, nullptr);

    // submit cmd buf to the queue and execute it.
    // _renderFence will now block until the commands finish.
    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

    VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));

}

void VulkanEngine::initImGui() {
    // create descriptor pool for imgui

    // NOTE the pool is very oversize, but its copied from the imgui demo.

    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                                          { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                                          { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                                          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                                          { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                                          { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &imguiPool));


    // Initialize Imgui.

    ImGui::CreateContext();

    ///NOTE i have no idea if this should be true or false ;-;
    ImGui_ImplGlfw_InitForVulkan(_window, true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = _instance;
    initInfo.PhysicalDevice = _chosenGPU;
    initInfo.Device = _device;
    initInfo.Queue = _graphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;
    initInfo.ColorAttachmentFormat = _swapchainImageFormat;

    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);

    ///NOTE the vkguide passes cmd here, but in the version im using its not needed???
    immediate_submit([&](VkCommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    _mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorPool(_device, imguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
}

void VulkanEngine::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment = VkInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingInfo renderInfo = VkInit::renderingInfo(_swapchainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

void VulkanEngine::initTrianglePipeline() {

    VkShaderModule triangleFragShader;
    if (!VkUtil::loadShaderModule("colored_triangle.frag.spv", _device, &triangleFragShader)) {
        spdlog::error("[Engine] error loading the triangle frag shader");
    } else {
        spdlog::info("[Engine] loaded the triangle frag shader");
    }

    VkShaderModule triangleVertexShader;
    if (!VkUtil::loadShaderModule("colored_triangle.vert.spv", _device, &triangleVertexShader)) {
        spdlog::error("[Engine] error loading the triangle vertex shader");
    } else {
        spdlog::info("[Engine] loaded the triangle vert shader");
    }

    VkPipelineLayoutCreateInfo pipelineLayout = VkInit::pipelineLayoutCreateInfo();
    VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayout, nullptr, &_trianglePipelineLayout));


    PipelineBuilder pipelineBuilder;

    pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

    // connect the vertex and frag to the pipeline
    pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);

    //draw Triangles
    pipelineBuilder.setInputToplogy(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Filled Triangles
    pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);

    //no backface culling
    pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    //No multisampling
    pipelineBuilder.setMultisamplingNone();

    //no blending
    pipelineBuilder.disableBlending();

    //no depthTest
    pipelineBuilder.disableDepthtest();

    // connect the draw image
    pipelineBuilder.setColorAttachmentFormat(_drawImage.imageFormat);
    pipelineBuilder.setDepthFormat(VK_FORMAT_UNDEFINED);

    //finally build the pipeline
    _trianglePipeline = pipelineBuilder.buildPipeline(_device);

    //clean
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);
    vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

    _mainDeletionQueue.pushFunction([&]() {
            vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
            vkDestroyPipeline(_device, _trianglePipeline, nullptr);
    });
}

void VulkanEngine::drawGeometry(VkCommandBuffer cmd) {
    spdlog::info("Drawing GEOMT!");
    //begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo colorAttachment = VkInit::attachmentInfo(_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);

    VkRenderingInfo renderInfo = VkInit::renderingInfo(_drawExtent, &colorAttachment, nullptr);
    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

    //set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = _drawExtent.width;
    viewport.height = _drawExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = _drawExtent.width;
    scissor.extent.height = _drawExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    //launch a draw command to draw 3 vertices
    vkCmdDraw(cmd, 3, 1, 0, 0);


    // PLEASE WORK I BEG YOOOOUUUU
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);

    GPUDrawPushConstants push_constants;
    push_constants.worldMatrix = glm::mat4{ 1.f };
    push_constants.vertexBuffer = rectangle.vertexBufferAddress;

    vkCmdPushConstants(cmd, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
    vkCmdBindIndexBuffer(cmd, rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);


    vkCmdEndRendering(cmd);
}

AllocatedBuffer VulkanEngine::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {

    //Allocate buffer
    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    AllocatedBuffer newBuffer;

    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaAllocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));


    return newBuffer;
    spdlog::info("creating buffer!");
}

void VulkanEngine::destroyBuffer(const AllocatedBuffer& buffer) {
    vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}

GPUMeshBuffers VulkanEngine::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
    spdlog::info("UploadingMesh!");
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;

    newSurface.vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                           VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = newSurface.vertexBuffer.buffer;
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(_device, &deviceAddressInfo);

    //create index buffer
    newSurface.indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();

    //copy vertex buffer
    memcpy(data, vertices.data(), vertexBufferSize);
    //copy index buffer
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

    immediate_submit([&](VkCommandBuffer cmd) {
       VkBufferCopy vertexCopy{ 0 };
       vertexCopy.dstOffset = 0;
       vertexCopy.srcOffset = 0;
       vertexCopy.size = vertexBufferSize;

       vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

       VkBufferCopy indexCopy{ 0 };
       indexCopy.dstOffset = 0;
       indexCopy.size = vertexBufferSize;
       indexCopy.size = indexBufferSize;

       vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
    });

    destroyBuffer(staging);

    return newSurface;
    spdlog::info("Uploaded the mesh!");
}

void VulkanEngine::initMeshPipeline() {
    spdlog::info("GOT TO INIT MESH pipeline");
    VkShaderModule triangleFragShader;
    if (!VkUtil::loadShaderModule("colored_triangle.frag.spv", _device, &triangleFragShader)) {
        spdlog::error("[Engine] error loading the triangle frag shader");
    } else {
        spdlog::info("[Engine] loaded the triangle frag shader");
    }

    VkShaderModule triangleVertexShader;
    if (!VkUtil::loadShaderModule("colored_triangle_mesh.vert.spv", _device, &triangleVertexShader)) {
        spdlog::error("[Engine] error loading the triangle vertex shader");
    } else {
        spdlog::info("[Engine] loaded the triangle vert shader");
    }

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info = VkInit::pipelineLayoutCreateInfo();
    pipeline_layout_info.pPushConstantRanges = &bufferRange;
    pipeline_layout_info.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_meshPipelineLayout));


    PipelineBuilder pipelineBuilder;

    //use the triangle layout we created
    pipelineBuilder._pipelineLayout = _meshPipelineLayout;
    //connecting the vertex and pixel shaders to the pipeline
    pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
    //it will draw triangles
    pipelineBuilder.setInputToplogy(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    //filled triangles
    pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
    //no backface culling
    pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    //no multisampling
    pipelineBuilder.setMultisamplingNone();
    //no blending
    pipelineBuilder.disableBlending();

    pipelineBuilder.disableDepthtest();

    //connect the image format we will draw into, from draw image
    pipelineBuilder.setColorAttachmentFormat(_drawImage.imageFormat);
    pipelineBuilder.setDepthFormat(VK_FORMAT_UNDEFINED);

    //finally build the pipeline
    _meshPipeline = pipelineBuilder.buildPipeline(_device);

    //clean structures
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);
    vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

    _mainDeletionQueue.pushFunction([&]() {
        vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);
        vkDestroyPipeline(_device, _meshPipeline, nullptr);
    });
    spdlog::info("Did the mesh pipeline!");
}

void VulkanEngine::initDefaultData() {
    spdlog::info("INITING DEFAULT DATA!");
    std::array<Vertex,4> rect_vertices;

    rect_vertices[0].position = {0.5,-0.5, 0};
    rect_vertices[1].position = {0.5,0.5, 0};
    rect_vertices[2].position = {-0.5,-0.5, 0};
    rect_vertices[3].position = {-0.5,0.5, 0};

    rect_vertices[0].color = {0,0, 0,1};
    rect_vertices[1].color = { 0.5,0.5,0.5 ,1};
    rect_vertices[2].color = { 1,0, 0,1 };
    rect_vertices[3].color = { 0,1, 0,1 };

    std::array<uint32_t,6> rect_indices;

    rect_indices[0] = 0;
    rect_indices[1] = 1;
    rect_indices[2] = 2;

    rect_indices[3] = 2;
    rect_indices[4] = 1;
    rect_indices[5] = 3;

    // just a simple rectangle for testing
    rectangle = uploadMesh(rect_indices,rect_vertices);
    spdlog::info("Done with default data");
}
