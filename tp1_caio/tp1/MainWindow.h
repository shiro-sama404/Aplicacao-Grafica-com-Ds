//[]---------------------------------------------------------------[]
//|                                                                 |
//| MainWindow.h                                                |
//|                                                                 |
//| Main graphics window for TP1                                   |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "graphics/GLRenderWindow3.h"
#include "PBRRenderer.h"
#include "RayCaster.h"
#include "SceneBuilder.h"
#include "GUIInitializer.h"

namespace cg { class GUIInitializer; }

namespace cg
{ // begin namespace cg

//
// MainWindow: janela gráfica principal
//
class MainWindow: public GLRenderWindow3
{
public:
  MainWindow(int width, int height):
    GLRenderWindow3{"TP1 - PBR Renderer", width, height},
    _scene{nullptr},
    _renderer{nullptr},
    _gui{nullptr} 
  {
    // Instancia a GUI passando a própria janela como referência
    _gui = new GUIInitializer(*this);
  }

  ~MainWindow()
  {
    delete _gui;
    delete _renderer;
    delete _scene;
  }

  bool windowResizeEvent(int width, int height) override;

  bool keyInputEvent(int, int, int) override;
  bool mouseButtonInputEvent(int button, int action, int mods) override;
  bool mouseMoveEvent(double xPos, double yPos) override;
  bool scrollEvent(double xoffset, double yoffset) override;
  bool onMouseLeftPress(int x, int y) override;

  Camera* camera();
  Scene* scene() { return _scene; }
  PBRActor* selectedActor() const { return _selectedActor; }
  void setSelectedActor(PBRActor* actor) { _selectedActor = actor; }
  void resetScene();
  
  // Controle de renderer
  bool useRayCaster() const { return _useRayCaster; }
  void setUseRayCaster(bool use) { _useRayCaster = use; }
  PBRRenderer* pbrRenderer() const { return _renderer; }
  RayCaster* rayCaster() const { return _rayCaster; }

protected:
  // Inicialização OpenGL
  void initialize() override;
  // Renderização da cena
  void render() override;
  // Interface gráfica (ImGui)
  void gui() override;
  // Finalização OpenGL
  void terminate() override;

private:
  Scene* _scene;
  PBRRenderer* _renderer;
  RayCaster* _rayCaster;
  GUIInitializer* _gui;
  PBRActor* _selectedActor = nullptr;
  bool _useRayCaster = false; // false = PBRRenderer, true = RayCaster

  bool _isMinimized = false;
  bool _isDragging = false;
  int _dragButton = -1;
  double _lastX = 0.0;
  double _lastY = 0.0;
}; // MainWindow

} // end namespace cg