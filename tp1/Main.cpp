//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2025 - Trabalho Prático 1                         |
//| Computação Gráfica - UFMS                                       |
//|                                                                 |
//[]---------------------------------------------------------------[]
//
// OVERVIEW: Main.cpp
// ========
// Main function for PBR rendering application.
//
// Author: Trabalho Prático 1
// Last revision: 06/11/2025

#include "graphics/Application.h"
#include "MainWindow.h"

using namespace cg;

int
main(int argc, char** argv)
{
  puts("==============================================");
  puts("  PBR Rendering - Trabalho Prático 1");
  puts("  Computação Gráfica - UFMS");
  puts("==============================================\n");
  
  puts("Camera controls:");
  puts("  (w) move forward    (s) move backward");
  puts("  (a) move left       (d) move right");
  puts("  (q) move up         (z) move down\n");
  
  puts("Mouse controls:");
  puts("  (scroll wheel)      zoom");
  puts("  (middle-click)      pan");
  puts("  (Alt+right-click)   rotate\n");
  
  puts("Features:");
  puts("  - PBR shading with 3 point lights");
  puts("  - Multiple sphere actors with varying materials");
  puts("  - Interactive GUI for lights and materials");
  puts("  - Real-time parameter adjustment\n");
  
  puts("Starting application...\n");
  
  return Application{new MainWindow{1280, 720}}.run(argc, argv);
}