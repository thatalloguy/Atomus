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
        Renderer/VkEngine.cpp
        Renderer/VkEngine.h)



target_link_libraries(AtomusVulkan PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(AtomusVulkan PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(AtomusVulkan PRIVATE spdlog)
target_link_libraries(AtomusVulkan PRIVATE fastgltf)


target_include_directories(AtomusVulkan PRIVATE "Thirdparty/imgui")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/stb")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/sdl/include")
