cmake_minimum_required(VERSION 3.15)
project(mod-hot-reloader VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(wil CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

add_executable(mod_hot_reloader
    main.cpp
    HotReloader.cpp
    Config.cpp
    Util.cpp
)

target_link_libraries(mod_hot_reloader PRIVATE WIL::WIL)
target_link_libraries(mod_hot_reloader PRIVATE utf8cpp::utf8cpp)
