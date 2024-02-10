cmake_minimum_required(VERSION 3.27)
project(AtomusVulkan)

set(CMAKE_CXX_STANDARD 17)


#libs go here :)
add_subdirectory("Thirdparty/vk-bootstrap")
add_subdirectory("Thirdparty/VulkanMemoryAllocator")
add_subdirectory("Thirdparty/spdlog")
add_subdirectory("Thirdparty/fastgltf")

#using this because im way to lazy to download it :)





add_executable(AtomusVulkan main.cpp
        # Engine related files
        Renderer/VkEngine.cpp
        Renderer/VkEngine.h
        Renderer/VkInitializers.cpp
        Renderer/VkInitializers.h
        Renderer/VkImages.cpp
        Renderer/VkImages.h
        Renderer/VkPipelines.cpp
        Renderer/VkPipelines.h
        Renderer/VkDescriptors.cpp
        Renderer/VkDescriptors.h
        Renderer/VkLoader.cpp
        Renderer/VkLoader.h
        Renderer/VkTypes.h)



target_link_libraries(AtomusVulkan PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(AtomusVulkan PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(AtomusVulkan PRIVATE spdlog)
target_link_libraries(AtomusVulkan PRIVATE fastgltf)


target_include_directories(AtomusVulkan PRIVATE "Thirdparty/imgui")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/stb")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/sdl/include")
