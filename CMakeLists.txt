cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(nJinnVulkan)

add_subdirectory(thirdparty)

add_subdirectory(meshConverterLib)
add_subdirectory(meshConverterApp)
