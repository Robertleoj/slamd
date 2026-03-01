cmake_minimum_required(VERSION 3.14)
project(myviz_bindings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(LIB_DIR vendored_deps)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

# Disable unnecessary sub-targets from vendored deps
set(OPTION_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPTION_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(OPTION_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if(APPLE)
    set(INSTALL_PATH "@loader_path")
else()
    set(INSTALL_PATH "$ORIGIN")
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)

# EXCLUDE_FROM_ALL prevents vendored deps from installing their own
# files (static archives, examples, tools) into the wheel.
add_subdirectory(slamd_src EXCLUDE_FROM_ALL)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module REQUIRED)

# Try to find installed pybind11
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND)
    message(STATUS "pybind11 not found, using FetchContent to grab it")
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        stable
    )
    FetchContent_MakeAvailable(pybind11)
endif()

set(BUILD_SHARED_LIBS ON)
set(PYBIND11_PYTHON_TARGET "Python::Module")
pybind11_add_module(bindings MODULE src/main.cpp)
set(BUILD_SHARED_LIBS OFF)

target_include_directories(
    bindings

    PRIVATE
        ${LIB_DIR}/glm
        ${LIB_DIR}/glbinding/include
        ${LIB_DIR}/glfw/include
)

target_link_libraries(bindings
    PUBLIC
        slamd::slamd
        slamd_common
        slamd_flatbuff

    PRIVATE
        imgui::imgui
        glfw
        glbinding::glbinding
        glbinding::glbinding-aux
        fmt::fmt
)

# slamd_window is excluded from ALL due to EXCLUDE_FROM_ALL on slamd_src,
# so we need to make bindings depend on it to ensure it gets built.
add_dependencies(bindings slamd_window)

install(TARGETS slamd_window DESTINATION slamd)
install(TARGETS bindings DESTINATION slamd)

# Strip installed binaries to reduce wheel size
set(CMAKE_INSTALL_DO_STRIP ON)
