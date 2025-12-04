#pragma once

#include "graphics/GLImage.h"
#include "graphics/GLRenderWindow3.h"
#include "PBRRenderer.h"
#include "RayCaster.h"
#include "SceneBuilder.h"
#include "GUIInitializer.h"

namespace cg { class GUIInitializer; }

namespace cg
{

// Classe principal responsável pelo gerenciamento da janela, contexto gráfico e integração dos subsistemas.
// Atua como controlador central, despachando eventos de entrada e coordenando as pipelines de renderização (Rasterização e Ray Tracing).
class MainWindow: public GLRenderWindow3
{
public:
  MainWindow(int width, int height):
    GLRenderWindow3{"TP1 - PBR Renderer", width, height},
    _scene{nullptr},
    _image{ nullptr },
    _renderer{nullptr},
    _gui{nullptr} 
  {
    _gui = new GUIInitializer(*this);
  }

  ~MainWindow()
  {
    delete _gui;
    delete _renderer;
    delete _scene;
  }

  // --- Gerenciamento de Eventos (Input/System) ---

  bool windowResizeEvent(int width, int height) override;
  bool keyInputEvent(int, int, int) override;
  bool mouseButtonInputEvent(int button, int action, int mods) override;
  bool mouseMoveEvent(double xPos, double yPos) override;
  bool scrollEvent(double xoffset, double yoffset) override;
  
  // Tratamento específico para seleção de objetos via mouse (Ray Picking).
  bool onMouseLeftPress(int x, int y) override;

  // --- Acesso e Controle de Estado ---
  Camera* getCamera();
  Scene* scene() { return _scene; }
  
  PBRActor* selectedActor() const { return _selectedActor; }
  void setSelectedActor(PBRActor* actor) { _selectedActor = actor; }
  
  // Reinicia a geometria da cena para o estado inicial.
  void resetScene();
  
  // Controle de alternância entre pipelines de renderização.
  bool useRayCaster() const { return _enableRayCaster; }
  void setUseRayCaster(bool use) { _enableRayCaster = use; }
  
  PBRRenderer* pbrRenderer() const { return _renderer; }
  RayCaster* rayCaster() const { return _rayCaster; }

protected:
  // Inicializa o estado da aplicação, carrega a cena e configura os renderizadores OpenGL/CPU.
  void initialize() override;

  // Atualiza o estado da aplicação.
  void update() override;
  
  // Executa o ciclo de renderização do frame atual.
  void render() override;
  
  // Renderiza a camada de interface gráfica (ImGui).
  void gui() override;
  
  // Libera recursos e finaliza o contexto gráfico.
  void terminate() override;

private:
  Reference<Scene> _scene;
  Reference<GLImage> _image;
  PBRRenderer* _renderer;   // Pipeline de Rasterização (OpenGL)
  RayCaster* _rayCaster;    // Pipeline de Ray Casting (CPU)
  GUIInitializer* _gui;
  Reference<PBRActor> _selectedActor = nullptr;
  
  uint32_t _cameraTimestamp = 0;
  
  bool _enableRayCaster = false; // Flag de controle do renderizador ativo.

  // Variáveis de estado para controle de câmera e input.
  bool _isMinimized = false;
  bool _isDragging = false;
  int _dragButton = -1;
  double _lastX = 0.0;
  double _lastY = 0.0;
};

}