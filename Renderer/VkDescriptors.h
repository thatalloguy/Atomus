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

class VkDescriptors {

};


#endif //ATOMUSVULKAN_VKDESCRIPTORS_H
