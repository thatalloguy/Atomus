//
// Created by allos on 2/10/2024.
//


/*
 * Purpose of this file for future reference:
 * Will contain descriptor set abstractions
 */

#ifndef ATOMUSVULKAN_VKDESCRIPTORS_H
#define ATOMUSVULKAN_VKDESCRIPTORS_H

#include "VkTypes.h"

struct DescriptorLayoutBuilder {

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void addBinding(uint32_t binding, VkDescriptorType type);
    void clear();

    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool pool;

    void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

class VkDescriptors {

};


#endif //ATOMUSVULKAN_VKDESCRIPTORS_H
