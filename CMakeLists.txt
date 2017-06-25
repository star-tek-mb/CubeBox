cmake_minimum_required(VERSION 3.0)

project(cubebox)

file(GLOB SOURCES cubebox/*.c)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(cubebox STATIC ${SOURCES})
target_link_libraries(cubebox m ${X11_LIBRARIES} ${OPENGL_LIBRARIES})
target_include_directories(cubebox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
