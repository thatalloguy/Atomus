cmake_minimum_required(VERSION 3.27)
project(AtomusVulkan)

set(CMAKE_CXX_STANDARD 17)

#libs go here :)
add_subdirectory("Thirdparty/vk-bootstrap")
add_subdirectory("Thirdparty/VulkanMemoryAllocator")
add_subdirectory("Thirdparty/sdl")





add_executable(AtomusVulkan main.cpp)

target_link_libraries(AtomusVulkan vk-bootstrap::vk-bootstrap)
target_link_libraries(AtomusVulkan GPUOpen::VulkanMemoryAllocator)
target_link_libraries(AtomusVulkan SDL2)