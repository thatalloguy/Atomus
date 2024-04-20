//
// Created by allos on 2/10/2024.
//


/*
 * Purpose of this file for future reference:
 * Will contain GLTF loading logic
 */

#pragma once

#include "VkTypes.h"
#include <unordered_map>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/util.hpp>


class VulkanEngine;

struct GLTFMaterial {
    MaterialInstance data;
};

struct GeoSurface {
    uint32_t startIndex;
    uint32_t count;
    std::shared_ptr<GLTFMaterial> material;
};


struct MeshAsset {
    std::string name;

    std::vector<GeoSurface> surfaces;
    GPUMeshBuffers meshBuffers;
};

namespace VkLoader {

    std::optional<std::vector<std::shared_ptr<MeshAsset>>>
    loadGltfMeshes(VulkanEngine *engine, std::filesystem::path filePath);

}
