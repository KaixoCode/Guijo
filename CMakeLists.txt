cmake_minimum_required (VERSION 3.0)
set(GUIJO "Guijo")
set(CMAKE_CXX_STANDARD 20)
set(GUIJO_SRC "${${GUIJO}_SOURCE_DIR}/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GUIJO_SRC}/cmake/")

project (${GUIJO})

add_library(GLAD STATIC libs/glad/glad.c)

find_package(OpenGL REQUIRED)

add_subdirectory(libs)

file(GLOB_RECURSE GUIJO_SOURCE
  "${GUIJO_SRC}source/*.cpp"
  "${GUIJO_SRC}include/*.hpp"
  "${GUIJO_SRC}include/*.shader"
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GUIJO_INCLUDE ${GUIJO_SRC}include/ ${GUIJO_SRC}libs/)

option(GUIJO_BUILD_EXAMPLE "Guijo Build Example" OFF)
option(GUIJO_BUILD_DOCS "Guijo Build Docs" OFF)

add_library(${GUIJO} STATIC ${GUIJO_SOURCE})

target_compile_definitions(${GUIJO} PUBLIC USE_OPENGL)
include_directories(${GUIJO} ${GUIJO_INCLUDE})
target_link_libraries(${GUIJO} PUBLIC
  ${OPENGL_LIBRARY} 
  GLAD
  freetype
  comctl32 
  propsys 
  dwmapi 
  gdiplus 
  user32
)
target_precompile_headers(${GUIJO} PRIVATE "${GUIJO_SRC}include/Guijo/pch.hpp")

source_group(TREE ${GUIJO_SRC} FILES ${GUIJO_SOURCE})

if (GUIJO_BUILD_EXAMPLE)
set(GUIJO_EXAMPLE_NAME "GuijoExample")

file(GLOB_RECURSE GUIJO_EXAMPLE_SOURCE 
  "${GUIJO_SRC}example/*.cpp"
)
add_executable(${GUIJO_EXAMPLE_NAME} ${GUIJO_EXAMPLE_SOURCE})
target_include_directories(${GUIJO_EXAMPLE_NAME} PRIVATE ${GUIJO_INCLUDE})
target_link_libraries(${GUIJO_EXAMPLE_NAME} PRIVATE ${GUIJO})
source_group(TREE ${GUIJO_SRC} FILES ${GUIJO_EXAMPLE_SOURCE})
endif()

if(GUIJO_BUILD_DOCS)
add_subdirectory("docs")
endif()