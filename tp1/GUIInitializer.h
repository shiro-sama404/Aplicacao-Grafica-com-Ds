#pragma once

#include "graphics/Application.h"
#include "graphics/Camera.h"
#include "PBRActor.h"

namespace cg
{

class MainWindow;

// Classe responsável pela inicialização e gerenciamento da Interface Gráfica do Usuário (GUI).
class GUIInitializer
{
public:
  GUIInitializer(MainWindow& window);

  // Renderiza os elementos da interface gráfica para o frame atual.
  void draw();

private:
  MainWindow& _window;
  int _selectedActorIndex = -1;
  int _resetCooldown = 0;

  // Métodos de construção de painéis da interface.
  void drawSceneControls();
  void drawRendererControls();
  void drawCameraControls();
  void drawLightControls();
  void drawMaterialControls();
  
  PBRActor* getSelectedActor();
};

}