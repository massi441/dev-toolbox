cmake_minimum_required(VERSION 3.15)
project(mod-project-creator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(../core ${CMAKE_CURRENT_BINARY_DIR}/core)

add_executable(project_creator
    main.cpp
)

target_link_libraries(project_creator PRIVATE ml_core)
