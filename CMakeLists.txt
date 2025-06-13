# CMakeList.txt : CMake project for vulkan_renderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project("vulkan_renderer")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(vulkan REQUIRED)
find_package(Stb REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (vulkan_renderer "src/main.cpp" 
								"src/Window.hpp" "src/Window.cpp" 
								"src/Graphics/Renderer.cpp" "src/Graphics/Renderer.hpp" 
								"src/Graphics/Vertex.hpp"   "src/Graphics/Camera.cpp" "src/Scene/GameObject.cpp"  
                                "src/Core/Factory.cpp" "src/Core/Engine.cpp" "src/Graphics/Renderable.cpp" 
                                "src/Core/Utils.cpp" "src/Resources/ResourceManager.cpp" "src/Scene/Scene.cpp" 
                                "src/Resources/ResourceImporter.cpp" "src/Tests/TestScene.cpp" "src/Graphics/GraphicsPipeline.cpp" 
                                "src/Graphics/CommandPool.cpp" "src/Graphics/CommandBuffer.cpp" "src/Transform/TransformComponent.cpp" "src/Scene/Component.cpp"  "src/Graphics/Light.cpp" "src/ImGui/ImGuiEditor.cpp"  "src/Graphics/RenderResources.cpp" "src/Graphics/ShadowPipeline.cpp" "src/Graphics/GraphicsUtils.cpp")

add_custom_command(
    TARGET vulkan_renderer PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/data
)

# TODO: Add tests and install targets if needed.
target_link_libraries(vulkan_renderer PRIVATE glm::glm glfw Vulkan::Vulkan tinyobjloader::tinyobjloader nlohmann_json::nlohmann_json)
target_link_libraries(vulkan_renderer PRIVATE unofficial::shaderc::shaderc)
target_link_libraries(vulkan_renderer PRIVATE imgui::imgui imguizmo::imguizmo)
target_include_directories(vulkan_renderer PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(vulkan_renderer PRIVATE ${CMAKE_SOURCE_DIR}/src)
