project(meshConverterLib)

add_library(${PROJECT_NAME}
    MeshData.cpp
    MeshLoader.cpp
    TypeConverter.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC half
)

target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
