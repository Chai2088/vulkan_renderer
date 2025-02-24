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

# Add source to this project's executable.
add_executable (vulkan_renderer "src/main.cpp" 
								"src/Window.hpp" "src/Window.cpp" 
								"src/Graphics/Renderer.cpp" "src/Graphics/Renderer.hpp" 
								"src/Graphics/Shader.hpp" "src/Graphics/Shader.cpp"
								"src/Graphics/Vertex.hpp"   "src/Graphics/Camera.cpp" "src/Scene/GameObject.cpp"  "src/Core/Factory.cpp" "src/Core/Engine.cpp" "src/Graphics/Renderable.cpp"  "src/Core/Utils.cpp" "src/Resources/ResourceManager.cpp" "src/Scene/Scene.cpp" "src/Resources/ResourceImporter.cpp" "src/Tests/TestScene.cpp")

add_custom_command(
    TARGET vulkan_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/data
)

# TODO: Add tests and install targets if needed.
target_link_libraries(vulkan_renderer PRIVATE glm::glm glfw Vulkan::Vulkan tinyobjloader::tinyobjloader)
target_include_directories(vulkan_renderer PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(vulkan_renderer PRIVATE ${CMAKE_SOURCE_DIR}/src)