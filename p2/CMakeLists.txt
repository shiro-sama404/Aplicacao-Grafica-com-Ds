cmake_minimum_required(VERSION 3.16)

project(p2 CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  message(STATUS "Forçando runtime do MSVC para: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()

set(CG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../cg")
set(CG_BUILD_DIR "${CMAKE_BINARY_DIR}/cg_build")

add_subdirectory(${CG_DIR} ${CG_BUILD_DIR})

set(GL3W_SRC "${CG_DIR}/externals/src/gl3w.c")

add_executable(p2
  Actor.h
  Actor.cpp
  Intersection.h
  Plane.cpp
  Plane.h
  Scene.cpp
  Scene.h
  Shape3.h
  Sphere.cpp
  Sphere.h
  Box.cpp
  Box.h
  Main.cpp
  MainWindow.cpp
  MainWindow.h
  ${GL3W_SRC}
)

target_include_directories(p2 PRIVATE
  ${CG_DIR}/include
  ${CG_DIR}/externals/include
  ${CG_DIR}/externals/include/GL
)

target_link_libraries(p2 PRIVATE
cg
opengl32
)

