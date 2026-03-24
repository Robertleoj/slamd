cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(slam_dunk VERSION 0.1.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED LIB_DIR)
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendored_deps)
endif()

if(NOT TARGET glbinding::glbinding-aux)
    add_subdirectory(${LIB_DIR}/glbinding ${CMAKE_CURRENT_BINARY_DIR}/_deps/glbinding)
endif()

if(NOT TARGET glfw)
    add_subdirectory(${LIB_DIR}/glfw ${CMAKE_CURRENT_BINARY_DIR}/_deps/glfw)
endif()

if(NOT TARGET spdlog::spdlog)
    add_subdirectory(${LIB_DIR}/spdlog ${CMAKE_CURRENT_BINARY_DIR}/_deps/spdlog)
endif()

if(NOT TARGET imgui::imgui)
    add_subdirectory(${LIB_DIR}/imgui_wrapper ${CMAKE_CURRENT_BINARY_DIR}/_deps/imgui_wrapper)
endif()

if(NOT TARGET fmt::fmt)
    add_subdirectory(${LIB_DIR}/fmt ${CMAKE_CURRENT_BINARY_DIR}/_deps/fmt)
endif()

if(NOT TARGET glm::glm)
    add_subdirectory(${LIB_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/_deps/glm)
endif()

if(NOT TARGET cxxopts)
    add_subdirectory(${LIB_DIR}/cxxopts ${CMAKE_CURRENT_BINARY_DIR}/_deps/cxxopts)
endif()

add_subdirectory(flatb)


# COMMON
add_library(slamd_common STATIC)
target_sources(
    slamd_common

    PRIVATE
        src/common/assert.cpp

        src/common/data/image.cpp
        src/common/data/mesh.cpp

        src/common/utils/mesh.cpp

        src/common/gmath/aabb.cpp
        src/common/gmath/angle.cpp
        src/common/gmath/transforms.cpp
        src/common/gmath/serialization.cpp
        src/common/gmath/misc.cpp
)

target_include_directories(
    slamd_common

    PUBLIC
        ${LIB_DIR}/glm
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_options(slamd_common PRIVATE -Wall -Wextra)


target_link_libraries(
    slamd_common

    PUBLIC
        spdlog::spdlog
        slamd_flatbuff
        cxxopts
        fmt::fmt

    PRIVATE
        spdlog::spdlog
)


# WINDOW
add_executable(slamd_window)

target_sources(
    slamd_window

    PRIVATE
        src/window/main.cpp

        src/window/image_texture.cpp
        src/window/glfw.cpp
        src/window/frame_buffer.cpp
        src/window/camera.cpp
        src/window/shaders.cpp
        src/window/arcball.cpp
        src/window/frame_timer.cpp
        src/window/run_window.cpp
        src/window/tree_overlay.cpp
        src/window/controls_hint.cpp
        src/window/state_manager.cpp
        src/window/connection.cpp
        src/window/message.cpp

        src/window/tree/tree.cpp
        src/window/tree/node.cpp
        src/window/tree/tree_path.cpp

        src/window/view/scene_view.cpp

        src/window/geom/geometry.cpp
        src/window/geom/deserialize.cpp
        src/window/geom/box.cpp
        src/window/geom/mesh.cpp
        src/window/geom/arrows.cpp
        src/window/geom/sphere.cpp
        src/window/geom/triad.cpp
        src/window/geom/poly_line.cpp
        src/window/geom/xy_grid.cpp
        src/window/geom/arcball_indicator.cpp
        src/window/geom/image.cpp
        src/window/geom/point_cloud.cpp
        src/window/geom/camera_frustum.cpp
        src/window/geom/mono_instanced.cpp
        src/window/geom/spheres.cpp
        src/window/geom/plane.cpp
)

target_include_directories(
    slamd_window

    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${LIB_DIR}/glm>
        $<BUILD_INTERFACE:${LIB_DIR}/asio/asio/include>
)

target_link_libraries(
    slamd_window

    PRIVATE
        glfw
        spdlog::spdlog
        slamd_flatbuff
        slamd_common
    PUBLIC
        glbinding::glbinding
        glbinding::glbinding-aux
        imgui::imgui
)

target_compile_options(slamd_window PRIVATE -Wall -Wextra)

# SLAMD
add_library(slamd STATIC)
add_library(slamd::slamd ALIAS slamd)

# pass the path of the window executable into the lib

target_compile_definitions(
    slamd

    PRIVATE
        EXEC_PATH=\"$<TARGET_FILE:slamd_window>\"
)

target_sources(
    slamd

    PRIVATE
        src/slamd/visualizer.cpp
        src/slamd/view.cpp
        src/slamd/spawn_window.cpp

        src/slamd/net/connection.cpp
        src/slamd/net/client_set.cpp

        src/slamd/tree/tree.cpp
        src/slamd/tree/tree_path.cpp

        src/slamd/geom/geometry.cpp
        src/slamd/geom/box.cpp
        src/slamd/geom/mesh.cpp
        src/slamd/geom/arrows.cpp
        src/slamd/geom/sphere.cpp
        src/slamd/geom/triad.cpp
        src/slamd/geom/poly_line.cpp
        src/slamd/geom/plane.cpp
        src/slamd/geom/image.cpp
        src/slamd/geom/point_cloud.cpp
        src/slamd/geom/camera_frustum.cpp
        src/slamd/geom/mono_instanced.cpp
        src/slamd/geom/spheres.cpp
)

target_include_directories(
    slamd

    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${LIB_DIR}/glm>
        $<BUILD_INTERFACE:${LIB_DIR}/asio/asio/include>
)

target_link_libraries(
    slamd

    PRIVATE
        spdlog::spdlog

    PUBLIC
        slamd_flatbuff
        slamd_common
)
target_compile_options(slamd PRIVATE -Wall -Wextra)
