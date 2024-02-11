//
// Created by allos on 2/10/2024.
//



/*
 * Purpose of this file for future reference:
 * This will contain helpers to create vulkan structures
 */


#pragma once

#include "VkTypes.h"

namespace VkInit {

    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/);

    VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags /*= 0*/);
    // I hope setting this to 0 doenst come and bite me in the ass later on :)
    VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/);
    VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);

    VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
    VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);
    VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
}

