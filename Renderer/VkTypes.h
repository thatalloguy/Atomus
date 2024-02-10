//
// Created by allos on 2/10/2024.
//


/*
 * Purpose of this file for future reference:
 * The entire codebase will include this header, it will provide widely used default structures and includes :)
 */

#ifndef ATOMUSVULKAN_VKTYPES_H
#define ATOMUSVULKAN_VKTYPES_H

// Include lib

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>




#define VK_CHECK(x)                                                      \
    do {                                                                 \
        VkResult err = x;                                                \
        if (err != VK_SUCCESS) {                                         \
            spdlog::error("Detected Vulkan error at {}: {}", __FILE__, __LINE__); \
            abort();                                                     \
        }                                                                \
    } while (0)

#endif //ATOMUSVULKAN_VKTYPES_H
