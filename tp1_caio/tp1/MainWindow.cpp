//[]---------------------------------------------------------------[]
//|                                                                 |
//| MainWindow.cpp                                              |
//|                                                                 |
//| Main graphics window implementation for TP1                    |
//|                                                                 |
//[]---------------------------------------------------------------[]

#include "MainWindow.h"
#include "PBRActor.h"
#include "imgui.h"
#include "GUIInitializer.h"

namespace cg
{ // begin namespace cg

void
MainWindow::initialize()
{
  _scene = SceneBuilder::buildDefaultScene();
  auto camera = new Camera;
  camera->setPosition({0, 0, 15});
  camera->setClippingPlanes(0.1f, 100.0f);
  camera->setProjectionType(Camera::Perspective);
  camera->setViewAngle(45.0f);
  camera->setEulerAngles({0, 0, 0});
  camera->setAspectRatio((float)width() / (float)height());

  // Criar renderer
  _renderer = new PBRRenderer{*_scene, *camera};
  _renderer->setImageSize(width(), height());

  // Criar ray caster para seleção
  _rayCaster = new RayCaster{*_scene, *camera};
  _rayCaster->setImageSize(width(), height());

  // Configurar OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Log de inicialização
  printf("MainWindow initialized\n");
  printf("Scene: %d actors, %d lights\n", 
         _scene->actorCount(), 
         _scene->lightCount());
}

void MainWindow::resetScene()
{
  // Salva estado da câmera atual para não perder a posição
  auto oldCam = _renderer->camera();
  vec3f pos = oldCam->position();
  vec3f angles = oldCam->eulerAngles();

  // Recria cena
  delete _renderer;
  delete _scene;
  
  _scene = SceneBuilder::buildDefaultScene();
  
  // Recria câmera e renderer
  auto camera = new Camera;
  camera->setPosition(pos);
  camera->setEulerAngles(angles);
  camera->setClippingPlanes(0.1f, 100.0f);
  camera->setProjectionType(Camera::Perspective);
  camera->setViewAngle(45.0f);
  camera->setAspectRatio((float)width() / (float)height());

  _renderer = new PBRRenderer{*_scene, *camera};
  _renderer->setImageSize(width(), height());
  
  // Recriar ray caster
  delete _rayCaster;
  _rayCaster = new RayCaster{*_scene, *camera};
  _rayCaster->setImageSize(width(), height());
  
  // Se RayCaster estiver ativo, reconstruir BVH
  if (_useRayCaster && _rayCaster)
    _rayCaster->rebuildBVH();
}

bool MainWindow::windowResizeEvent(int width, int height)
{
    // Se a área for zero, a janela está minimizada
    if (width == 0 || height == 0)
    {
        _isMinimized = true;
        return true;
    }
    _isMinimized = false;

    // Atualiza o viewport do renderer para o novo tamanho
    if (_renderer)
        _renderer->setImageSize(width, height);
    if (_rayCaster)
        _rayCaster->setImageSize(width, height);

    auto cam = camera();
    if (cam)
        cam->setAspectRatio((float)width / (float)height);

    return GLWindow::windowResizeEvent(width, height);
}

bool MainWindow::keyInputEvent(int key, int action, int mods)
{
    // Se o ImGui estiver usando o teclado, ignoramos
    if (ImGui::GetIO().WantCaptureKeyboard) return false;
    
    // Apenas processamos eventos de PRESS ou REPEAT (segurar tecla)
    if (action == GLFW_RELEASE) return false;

    auto cam = camera();
    if (!cam) return false;

    // Velocidade de movimento
    const float speed = 0.5f; 
    vec3f d = vec3f::null();

    switch (key)
    {
        case GLFW_KEY_W: d.z -= speed; break; // Frente
        case GLFW_KEY_S: d.z += speed; break; // Trás
        case GLFW_KEY_A: d.x -= speed; break; // Esquerda
        case GLFW_KEY_D: d.x += speed; break; // Direita
        case GLFW_KEY_Q: d.y += speed; break; // Cima
        case GLFW_KEY_Z: d.y -= speed; break; // Baixo
        default: return false; // Tecla não tratada
    }

    // Aplica translação local à câmera
    cam->translate(d);
    return true;
}

bool MainWindow::mouseButtonInputEvent(int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    // Para botão esquerdo, deixar a classe base processar (ela chama onMouseLeftPress)
    // Se onMouseLeftPress retornar true (selecionou algo), a classe base não inicia arraste
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // A classe base vai chamar onMouseLeftPress
        bool handled = GLRenderWindow3::mouseButtonInputEvent(button, action, mods);
        
        // Se não selecionou nada e pressionou, permitir arraste para pan
        if (action == GLFW_PRESS && !handled)
        {
            _isDragging = true;
            _dragButton = button;
            int x, y;
            cursorPosition(x, y);
            _lastX = (double)x;
            _lastY = (double)y;
        }
        else if (action == GLFW_RELEASE)
        {
            _isDragging = false;
            _dragButton = -1;
        }
        
        return handled;
    }
    
    // Para outros botões, tratar arraste normalmente
    if (action == GLFW_PRESS)
    {
        _isDragging = true;
        _dragButton = button;
        int x, y;
        cursorPosition(x, y);
        _lastX = (double)x;
        _lastY = (double)y;
    }
    else if (action == GLFW_RELEASE)
    {
        _isDragging = false;
        _dragButton = -1;
    }

    // Para botão direito e meio, usar a classe base
    return GLRenderWindow3::mouseButtonInputEvent(button, action, mods);
}

bool MainWindow::mouseMoveEvent(double xPos, double yPos)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    // Se estiver arrastando com botão esquerdo e não selecionou nada, fazer pan
    if (_isDragging && _dragButton == GLFW_MOUSE_BUTTON_LEFT)
    {
        auto cam = camera();
        if (!cam) return false;

        // Calcula delta do movimento
        float dx = (float)(xPos - _lastX);
        float dy = (float)(yPos - _lastY);

        // Atualiza última posição
        _lastX = xPos;
        _lastY = yPos;

        if (dx == 0 && dy == 0) return true;

        // Pan (Mover câmera lateralmente)
        float panSpeed = cam->distance() * 0.002f;
        cam->translate(-dx * panSpeed, dy * panSpeed, 0.0f);
        return true;
    }
    
    // Para outros botões, usar a lógica padrão da classe base
    // Mas ainda precisamos tratar botão direito e meio
    if (_isDragging)
    {
        auto cam = camera();
        if (!cam) return false;

        float dx = (float)(xPos - _lastX);
        float dy = (float)(yPos - _lastY);

        _lastX = xPos;
        _lastY = yPos;

        if (dx == 0 && dy == 0) return true;

        // Botão Direito: Orbit (Girar câmera)
        if (_dragButton == GLFW_MOUSE_BUTTON_RIGHT)
        {
            const float sensitivity = 0.5f;
            cam->azimuth(-dx * sensitivity);
            cam->elevation(-dy * sensitivity);
        }
        // Botão Meio: Pan
        else if (_dragButton == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            float panSpeed = cam->distance() * 0.002f;
            cam->translate(-dx * panSpeed, dy * panSpeed, 0.0f);
        }
        return true;
    }

    return false;
}

bool MainWindow::scrollEvent(double xOffset, double yOffset)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    auto cam = camera();
    if (cam)
    {
        // Zoom in/out
        // Fator 1.1 dá um zoom suave
        float zoomFactor = (yOffset > 0) ? 1.1f : 0.9f;
        cam->zoom(zoomFactor);
    }
    return true;
}

bool MainWindow::onMouseLeftPress(int x, int y)
{
    // Selecionar ator quando RayCaster estiver DESATIVADO (modo OpenGL)
    // O RayCaster é usado apenas para fazer o ray casting de seleção
    if (ImGui::GetIO().WantCaptureMouse) return false;
    
    if (_rayCaster == nullptr) 
    {
        printf("RayCaster is null!\n");
        return false;
    }
    
    // Inverter coordenada Y (OpenGL tem origem no canto inferior esquerdo)
    int glY = height() - y;
    
    printf("Trying to select at position (%d, %d) -> (%d, %d)\n", x, y, x, glY);
    
    // Selecionar ator através de ray casting
    // O RayCaster sempre existe e pode ser usado para seleção, mesmo quando não está ativo para renderização
    _selectedActor = _rayCaster->selectActor(x, glY);
    
    // Atualizar PBRRenderer com o ator selecionado para feedback visual
    if (_renderer)
    {
        _renderer->setSelectedActor(_selectedActor);
    }
    
    // Atualizar GUI com ator selecionado
    if (_selectedActor)
    {
        printf("Selected actor: %s\n", _selectedActor->name());
        // Se selecionou um ator, retornar true para impedir arraste
        return true;
    }
    else
    {
        printf("No actor selected at position (%d, %d)\n", x, glY);
    }
    
    // Se não selecionou nada, permitir que o arraste funcione
    return false;
}

Camera*
MainWindow::camera()
{
  if (_useRayCaster && _rayCaster)
    return _rayCaster->camera();
  else if (_renderer)
    return _renderer->camera();
  return nullptr;
}

void
MainWindow::render()
{
  if (_isMinimized) return;
  
  // Usar o renderer ativo baseado na opção
  // Se RayCaster estiver ativo, desativar PBRRenderer
  if (_useRayCaster)
  {
    // RayCaster está ativo - PBRRenderer desativado
    // Por enquanto, RayCaster é usado apenas para seleção
    // A renderização visual ainda usa PBRRenderer, mas isso pode ser mudado
    // para renderizar imagem completa com ray casting se necessário
    if (_renderer != nullptr)
      _renderer->render();
  }
  else
  {
    // PBRRenderer está ativo - RayCaster desativado
    if (_renderer != nullptr)
      _renderer->render();
  }
}

void
MainWindow::gui()
{
  if (_gui) _gui->draw();
}

void
MainWindow::terminate()
{
  printf("MainWindow terminated\n");
}

} // end namespace cg