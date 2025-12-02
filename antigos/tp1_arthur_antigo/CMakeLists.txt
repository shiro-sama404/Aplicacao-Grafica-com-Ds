cmake_minimum_required(VERSION 3.16)

project(tp1 CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Forçar runtime do MSVC (importante para evitar conflitos de linking)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  message(STATUS "Forçando runtime do MSVC para: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()

# Definir diretórios do cg (biblioteca do professor)
set(CG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../cg")
set(CG_BUILD_DIR "${CMAKE_BINARY_DIR}/cg_build")

# Adicionar cg como subdirectory
add_subdirectory(${CG_DIR} ${CG_BUILD_DIR})

# Arquivo gl3w.c necessário para OpenGL
set(GL3W_SRC "${CG_DIR}/externals/src/gl3w.c")

# Lista de arquivos do projeto
add_executable(tp1
  PBRActor.h
  GUIInitializer.h
  GUIInitializer.cpp
  MainWindow.h
  MainWindow.cpp
  Main.cpp
  PBRRenderer.h
  PBRRenderer.cpp
  PBRMaterial.h
  SceneBuilder.h
  Scene.h
  Shape3.h
  Plane.h
  Sphere.h
  Cube.h
  ${GL3W_SRC}
)

# Incluir diretórios de headers
target_include_directories(tp1 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CG_DIR}/include
  ${CG_DIR}/externals/include
  ${CG_DIR}/externals/include/GL
)

# Linkar bibliotecas
target_link_libraries(tp1 PRIVATE
  cg
  opengl32
)

# Output no diretório do projeto
set_target_properties(tp1 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)