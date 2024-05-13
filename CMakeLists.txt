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

# Add source to this project's executable.
add_executable (vulkan_renderer "src/main.cpp" 
								"src/Window.hpp" "src/Window.cpp" 
								"src/Renderer.cpp" "src/Renderer.hpp" "src/Shader.hpp" "src/Shader.cpp")

# TODO: Add tests and install targets if needed.
target_link_libraries(vulkan_renderer PRIVATE glm::glm glfw Vulkan::Vulkan)
