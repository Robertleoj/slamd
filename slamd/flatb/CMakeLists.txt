add_compile_options(-Wno-stringop-overflow)

if (NOT TARGET flatbuffers)
    set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_FLATC OFF CACHE BOOL "" FORCE)
    add_subdirectory(${LIB_DIR}/flatbuffers ${CMAKE_CURRENT_BINARY_DIR}/_deps/flatbuffers)
endif()

# Grab all .cpp and .h/.hpp files in current dir
file(GLOB_RECURSE FLATBUFFERS_SRC "*.cpp" "*.h" "*.hpp")

# Make a static lib from 'em
add_library(slamd_flatbuff STATIC ${FLATBUFFERS_SRC})

target_link_libraries(slamd_flatbuff PUBLIC flatbuffers)


set_target_properties(slamd_flatbuff PROPERTIES LINKER_LANGUAGE CXX)

# Include current dir for headers
target_include_directories(slamd_flatbuff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
