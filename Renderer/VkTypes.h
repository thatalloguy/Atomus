//
// Created by allos on 2/10/2024.
//


/*
 * Purpose of this file for future reference:
 * The entire codebase will include this header, it will provide widely used default structures and includes :)
 */

#pragma once
// Include lib

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>

#include <thread>
#include <vulkan/vulkan.h>

#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include <spdlog/spdlog.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>


struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void pushFunction(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {

        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)();
        }
    }
};

struct AllocatedImage {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};


#define VK_CHECK(x)                                                      \
    do {                                                                 \
        VkResult err = x;                                                \
        if (err != VK_SUCCESS) {                                         \
            spdlog::info("ruh roh");                                                             \
            spdlog::error("Detected Vulkan error at {}: {}", __FILE__, __LINE__); \
            abort();                                                     \
        }                                                          \
    } while (0)
