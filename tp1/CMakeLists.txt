cmake_minimum_required(VERSION 3.16)

project(myapp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(myapp
  Main.cpp
  MainWindow.cpp
)

target_link_libraries(myapp cg)

set_target_properties(myapp PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
