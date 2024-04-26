//
// Created by allos on 2/10/2024.
//

#define GLM_ENABLE_EXPERIMENTAL
#include "VkLoader.h"

#include <stb_image.h>
#include "VkEngine.h"
#include "VkInitializers.h"
#include "VkTypes.h"


namespace VkLoader {

    std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(VulkanEngine* engine, std::filesystem::path filePath) {
        spdlog::info("Loading gltf: {}", filePath.string());

        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filePath);

        constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

        fastgltf::Asset gltf;
        fastgltf::Parser parser{};

        auto load = parser.loadGltfBinary(&data, filePath.parent_path(), gltfOptions);

        if (load) {
            gltf = std::move(load.get());
        } else {
            spdlog::error("Failed to load gltf: {}", fastgltf::to_underlying(load.error()));
            return {};
        }

        std::vector<std::shared_ptr<MeshAsset>> meshes;

        // use the same vector for all meshes so that mem doenst reallocate as often.
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        for (fastgltf::Mesh& mesh : gltf.meshes) {
            MeshAsset newMesh;

            newMesh.name = mesh.name;

            //clear the mesh arratys.
            indices.clear();
            vertices.clear();

            for (auto&& p : mesh.primitives) {
                GeoSurface newSurface;
                newSurface.startIndex = (uint32_t) indices.size();
                newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

                size_t initial_vtx = vertices.size();

                //load indexes
                {
                    fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];

                    indices.reserve(indices.size() + indexAccessor.count);

                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor, [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                    });
                }

                //load vertex positions
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 v, size_t index) {
                       Vertex newvtx;
                       newvtx.position = v;
                       newvtx.normal = { 1, 0, 0};
                       newvtx.color = glm::vec4{ 1.f };
                       newvtx.uv_x = 0;
                       newvtx.uv_y = 0;
                       vertices[initial_vtx + index] = newvtx;
                    });
                }

                //load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second], [&](glm::vec3 v, size_t index) {
                       vertices[initial_vtx + index].normal = v;
                    });
                }

                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {

                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index) {
                            vertices[initial_vtx + index].uv_x = v.x;
                            vertices[initial_vtx + index].uv_y = v.y;
                    });
                }

                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {

                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second], [&](glm::vec4 v, size_t index) {
                            vertices[initial_vtx + index].color = v;
                    });
                }
                newMesh.surfaces.push_back(newSurface);
            }

            //display the vertex normals

            constexpr bool OverrideColors = true;

            if (OverrideColors) {
                for (Vertex& vtx : vertices) {
                    vtx.color = glm::vec4(vtx.normal, 1.f);
                }
            }

            newMesh.meshBuffers = engine->uploadMesh(indices, vertices);


            meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newMesh)));
        }

        return meshes;
    }

    std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltf(VulkanEngine *engine, std::string_view filePath) {
        spdlog::info("Loading GLTF file: {}", filePath);

        std::shared_ptr<LoadedGLTF> scene = std::make_shared<LoadedGLTF>();
        scene->creator = engine;
        LoadedGLTF& file = *scene.get();

        fastgltf::Parser parser{};

        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;
        // fastgltf::Options::LoadExternalImages;

        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filePath);

        fastgltf::Asset gltf;

        std::filesystem::path path = filePath;

        auto type = fastgltf::determineGltfFileType(&data);
        if (type == fastgltf::GltfType::glTF) {
            auto load = parser.loadGltf(&data, path.parent_path(), gltfOptions);
            if (load) {
                gltf = std::move(load.get());
            } else {
                spdlog::error("Failed to load gltf file: {} | reason: {}", filePath, fastgltf::to_underlying(load.error()));
                return {};
            }
        } else if (type == fastgltf::GltfType::GLB) {
            auto load = parser.loadGltfBinary(&data, path.parent_path(), gltfOptions);
            if (load) {
                gltf = std::move(load.get());
            } else {
                spdlog::error("Failed to load gltf file: {} | reason: {}", filePath, fastgltf::to_underlying(load.error()));
                return {};
            }
        } else {
            spdlog::error("Failed to determine GLTF container");
            return {};
        }


        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
        };

        file.descriptorPool.init(engine->_device, gltf.materials.size(), sizes);
    }

    VkFilter extractFilter(fastgltf::Filter filter) {
        switch (filter) {
            //nearest samplers
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::NearestMipMapLinear:
                return VK_FILTER_LINEAR;

            // linear samplers
            case fastgltf::Filter::Linear:
            case fastgltf::Filter::LinearMipMapNearest:
            case fastgltf::Filter::LinearMipMapLinear:
            default:
                return VK_FILTER_LINEAR;
        }
    }
}