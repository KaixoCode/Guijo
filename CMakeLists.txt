cmake_minimum_required (VERSION 3.0)
set(GUIJO "Guijo")
set(CMAKE_CXX_STANDARD 20)
set(GUIJO_SRC "${${GUIJO}_SOURCE_DIR}/")

project (${GUIJO})

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE GUIJO_SOURCE
  "${GUIJO_SRC}source/*.cpp"
  "${GUIJO_SRC}include/*.hpp"
)

set(GUIJO_INCLUDE ${GUIJO_SRC}include/)

option(GUIJO_BUILD_EXAMPLE "Guijo Build Example" OFF)

add_library(${GUIJO} STATIC ${GUIJO_SOURCE})

target_include_directories(${GUIJO} PRIVATE ${GUIJO_INCLUDE})

target_link_libraries(${GUIJO} PRIVATE
	${OPENGL_LIBRARY} 
)

source_group(TREE ${GUIJO_SRC} FILES ${GUIJO_SOURCE})
target_precompile_headers(${GUIJO} PRIVATE "${GUIJO_SRC}include/Guijo/pch.hpp")

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
