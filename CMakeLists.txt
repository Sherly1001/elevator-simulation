cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(elevator-simulation C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

set(SRC src)
set(CMAKE_C_FLAGS "-Wall -Wextra")

set(LIBS
    ${GTK3_LIBRARIES}
)

link_directories(${GTK3_LIBRARY_DIRS})

include_directories(include
    ${GTK3_INCLUDE_DIRS}
)

file(GLOB_RECURSE SRC_FILES "${SRC}/*.c")

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
