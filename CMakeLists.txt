cmake_minimum_required(VERSION 4.0)

project(BeanDip C)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SRC_FILES})


add_executable(BeanDip ${SRC_FILES})
target_sources(BeanDip PRIVATE)


target_include_directories(BeanDip PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
)

