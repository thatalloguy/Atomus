//
// Created by allos on 2/10/2024.
//

#include "VkPipelines.h"
#include <spdlog/spdlog.h>

bool VkUtil::loadShaderModule(const char *filePath, VkDevice device, VkShaderModule *outShaderModule) {

    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("Could not open file: {}", filePath);
        return false;
    } else {
        spdlog::debug("Reading Shader file: {}", filePath);
    }


    size_t fileSize = (size_t)file.tellg();

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);

    file.read((char*)buffer.data(), fileSize);

    file.close();

    // Now do the shader stuff :)
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    // Check if we did it ;-;
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;

}
