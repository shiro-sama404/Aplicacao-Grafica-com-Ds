cmake_minimum_required(VERSION 3.16)

project(tp2 CXX C)

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

add_executable(tp2
  Main.cpp
  MainWindow.cpp
  RayTracer.cpp
  SceneWriter.cpp
  Writer.cpp
  reader/AbstractParser.cpp
  reader/Buffer.cpp
  reader/ErrorHandler.cpp
  reader/Expression.cpp
  reader/FileBuffer.cpp
  reader/ReaderBase.cpp
  reader/SceneReader.cpp
  reader/Scope.cpp
  ${GL3W_SRC}
)

# Incluir diretórios de headers
target_include_directories(tp2 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CG_DIR}/include
  ${CG_DIR}/externals/include
  ${CG_DIR}/externals/include/GL
)

target_link_libraries(tp2 PRIVATE
  cg
  opengl32
)

set_target_properties(tp2 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Copy assets to output directory after build
add_custom_command(TARGET tp2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying assets..."
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:tp2>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:tp2>/assets"
)
