//
// Created by allos on 2/10/2024.
//



/*
 * Purpose of this file for future reference:
 * This will contain helpers to create vulkan structures
 */


#ifndef ATOMUSVULKAN_VKINITIALIZERS_HPP
#define ATOMUSVULKAN_VKINITIALIZERS_HPP


#include "VkTypes.h"

namespace VkInit {

    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/) {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;
        return info;
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/) {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;

        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        return info;
    }

}



#endif //ATOMUSVULKAN_VKINITIALIZERS_HPP
