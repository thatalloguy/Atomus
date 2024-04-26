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

        //load samplers
        for (fastgltf::Sampler& sampler : gltf.samplers) {

            VkSamplerCreateInfo sampl { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr};
            sampl.maxLod = VK_LOD_CLAMP_NONE;
            sampl.minLod = 0;

            sampl.magFilter = extractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
            sampl.minFilter = extractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

            sampl.mipmapMode = extractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

            VkSampler newSampler;
            VK_CHECK(vkCreateSampler(engine->_device, &sampl, nullptr, &newSampler));

            file.samplers.push_back(newSampler);
        }

        // temp arrays for all the objects
        std::vector<std::shared_ptr<MeshAsset>>    meshes;
        std::vector<std::shared_ptr<Node>>         nodes;
        std::vector<AllocatedImage>                images;
        std::vector<std::shared_ptr<GLTFMaterial>> materials;

        //load all images

        for (fastgltf::Image& image : gltf.images) {

            images.push_back(engine->_errorImage);
        }

        //create buffer for holding the material info
        file.materialDataBuffer = engine->createBuffer(sizeof(GLTFMetallic_roughness::MaterialConstants) * gltf.materials.size(),
                                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        int data_index = 0;
        GLTFMetallic_roughness::MaterialConstants* sceneMaterialConstants = (GLTFMetallic_roughness::MaterialConstants*) file.materialDataBuffer.info.pMappedData;


        for (fastgltf::Material& mat : gltf.materials) {
            std::shared_ptr<GLTFMaterial> newMat = std::make_shared<GLTFMaterial>();
            materials.push_back(newMat);

            GLTFMetallic_roughness::MaterialConstants constants;
            constants.colorFactors.x = mat.pbrData.baseColorFactor[0];
            constants.colorFactors.y = mat.pbrData.baseColorFactor[1];
            constants.colorFactors.z = mat.pbrData.baseColorFactor[2];
            constants.colorFactors.w = mat.pbrData.baseColorFactor[3];

            constants.metalRoughFactors.x = mat.pbrData.metallicFactor;
            constants.metalRoughFactors.y = mat.pbrData.roughnessFactor;

            //write material parameters to buffer
            sceneMaterialConstants[data_index] = constants;

            MaterialPass passType = MaterialPass::MainColor;
            if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
                passType = MaterialPass::Transparent;
            }

            GLTFMetallic_roughness::MaterialResources materialResources;
            // default material textures
            materialResources.colorImage = engine->_whiteImage;
            materialResources.colorSampler = engine->_defaultSamplerLinear;
            materialResources.metalRoughImage = engine->_whiteImage;
            materialResources.metalRoughSampler = engine->_defaultSamplerLinear;

            //set the uniform buf for the mat data;
            materialResources.dataBuffer = file.materialDataBuffer.buffer;
            materialResources.dataBufferOffset = data_index * sizeof(GLTFMetallic_roughness::MaterialConstants);

            //grab textures from gltf file
            if (mat.pbrData.baseColorTexture.has_value()) {
                //yoink
                size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
                size_t sampler = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();

                materialResources.colorImage = images[img];
                materialResources.colorSampler = file.samplers[sampler];
            }

            //build material
            newMat->data = engine->metalRoughMaterial.writeMaterial(engine->_device, passType, materialResources, file.descriptorPool);

            data_index++;
        }

        // use the same arrays for all meshes so that we dont reallocate as often :)

        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        for (fastgltf::Mesh& mesh : gltf.meshes) {
            std::shared_ptr<MeshAsset> newMesh = std::make_shared<MeshAsset>();
            meshes.push_back(newMesh);

            file.meshes[mesh.name.c_str()] = newMesh;
            newMesh->name = mesh.name;

            //clear the mesh arrays on each mesh, we dont want to merge them ;)
            indices.clear();
            vertices.clear();

            for (auto&& p : mesh.primitives) {
                GeoSurface newSurface;
                newSurface.startIndex = (uint32_t)indices.size();
                newSurface.count = (uint32_t) gltf.accessors[p.indicesAccessor.value()].count;

                size_t initial_vtx = vertices.size();

                //load indexes
                {
                    fastgltf::Accessor& indexAcessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexAcessor.count);

                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAcessor,
                                                             [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                    });
                }

                //load vertex positions;
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                        [&](glm::vec3 v, size_t index){
                                    Vertex newVtx;
                                    newVtx.position = v;
                                    newVtx.normal = {1, 0, 0};
                                    newVtx.color = glm::vec4{1.f};
                                    newVtx.uv_x = 0;
                                    newVtx.uv_y = 0;
                                    vertices[initial_vtx + index] = newVtx;
                    });
                }

                // load normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf,
                                                                  gltf.accessors[(*normals).second],
                          [&](glm::vec3 v, size_t index){
                                    vertices[initial_vtx + index].normal = v;
                    });
                }

                //load uv's
                auto uv = p.findAttribute("TEXCOORD_)");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf,
                                                                  gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index){
                                                                      vertices[initial_vtx + index].uv_x = v.x;
                                                                      vertices[initial_vtx + index].uv_y = v.y;
                                                                  });
                }

                //load vertex colors
                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf,
                                                                  gltf.accessors[(*colors).second],
                                                                  [&](glm::vec4 v, size_t index){
                                                                      vertices[initial_vtx + index].color = v;
                                                                  });
                }
            }
        }
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

    VkSamplerMipmapMode extractMipmapMode(fastgltf::Filter filter) {
        switch (filter) {
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::LinearMipMapNearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;

            case fastgltf::Filter::NearestMipMapLinear:
            case fastgltf::Filter::LinearMipMapLinear:
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }
}