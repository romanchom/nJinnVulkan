project(meshConverterApp)

add_executable(${PROJECT_NAME}
    meshConverterApp.cpp
)

find_package(Boost COMPONENTS program_options iostreams)

target_link_libraries(${PROJECT_NAME}
    PUBLIC meshConverterLib Boost::program_options Boost::iostreams
)
