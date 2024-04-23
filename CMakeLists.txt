cmake_minimum_required(VERSION 3.27)
project(AtomusVulkan)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)


add_subdirectory("Thirdparty/vk-bootstrap")
add_subdirectory("Thirdparty/VulkanMemoryAllocator")
add_subdirectory("Thirdparty/spdlog")
add_subdirectory("Thirdparty/fastgltf")
add_subdirectory("Thirdparty/glfw")

file(GLOB_RECURSE IMGUI_SOURCE
        Thirdparty/imgui/imgui.cpp
        Thirdparty/imgui/imgui.h
        Thirdparty/imgui/imgui_internal.h
        Thirdparty/imgui/imgui_internal.cpp
        Thirdparty/imgui/imconfig.h
        Thirdparty/imgui/imstb_rectpack.h
        Thirdparty/imgui/imgui_draw.cpp
        Thirdparty/imgui/imgui_tables.cpp
        Thirdparty/imgui/imgui_widgets.cpp
        Thirdparty/imgui/imgui_demo.cpp
        Thirdparty/imgui/backends/imgui_impl_vulkan.h
        Thirdparty/imgui/backends/imgui_impl_vulkan.cpp
        Thirdparty/imgui/backends/imgui_impl_glfw.h
        Thirdparty/imgui/backends/imgui_impl_glfw.cpp
)

file(GLOB_RECURSE FASTGLTF_DEPS
        Thirdparty/fastgltf/deps/simdjson/simdjson.h
        Thirdparty/fastgltf/deps/simdjson/simdjson.cpp
)

add_executable(AtomusVulkan ${IMGUI_SOURCE} ${FASTGLTF_DEPS} main.cpp
        # Engine related files
        Renderer/VkEngine.cpp
        Renderer/VkEngine.h
        Renderer/VkInitializers.h
        Renderer/VkImages.cpp
        Renderer/VkImages.h
        Renderer/VkPipelines.cpp
        Renderer/VkPipelines.h
        Renderer/VkDescriptors.cpp
        Renderer/VkDescriptors.h
        Renderer/VkLoader.cpp
        Renderer/VkLoader.h
        Renderer/VkTypes.h
        Renderer/VkInitializers.cpp
        Renderer/Camera.cpp
        Renderer/Camera.h


)




target_link_libraries(AtomusVulkan PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(AtomusVulkan PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(AtomusVulkan PRIVATE spdlog)
target_link_libraries(AtomusVulkan PRIVATE fastgltf)
target_link_libraries(AtomusVulkan PRIVATE glfw Vulkan::Vulkan)



target_include_directories(AtomusVulkan PRIVATE "Thirdparty/imgui")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/stb")
target_include_directories(AtomusVulkan PRIVATE "Thirdparty/glm")
