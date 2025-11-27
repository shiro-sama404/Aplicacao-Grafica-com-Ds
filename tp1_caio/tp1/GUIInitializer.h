//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2025.                                             |
//|                                                                 |
//[]---------------------------------------------------------------[]
#pragma once

#include "graphics/Application.h" // Traz ImGui
#include "graphics/Camera.h"
#include "PBRActor.h"

namespace cg {

class MainWindow;

class GUIInitializer
{
public:
  GUIInitializer(MainWindow& window);
  void draw();

private:
  MainWindow& _window;
  int _selectedActorIndex = -1;

  void drawSceneControls();
  void drawRendererControls();
  void drawCameraControls();
  void drawLightControls();
  void drawMaterialControls();
  void drawActorInspector();
  
  // Helper
  PBRActor* getSelectedActor();
};

} // namespace cg