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

  // Métodos de construção de painéis específicos da interface.
  void drawSceneControls();
  void drawRendererControls();
  void drawCameraControls();
  void drawLightControls();
  void drawMaterialControls();
  void drawActorInspector();
  
  // Recupera o ator atualmente selecionado, priorizando a seleção via Ray Picking (mouse)
  // sobre a seleção via lista (combo box).
  PBRActor* getSelectedActor();
};

}