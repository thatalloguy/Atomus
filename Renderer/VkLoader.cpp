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

            //TODO DONT FORGET TO DELETE THIS!!!!!
            newMesh.meshBuffers = engine->uploadMesh(indices, vertices);


            meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newMesh)));
        }

        return meshes;
    }
}