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

    auto cam = _renderer ? _renderer->camera() : nullptr;
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

    if (action == GLFW_PRESS)
    {
        // Se for botão esquerdo, tentar selecionar ator
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            int x, y;
            cursorPosition(x, y);
            if (onMouseLeftPress(x, y))
            {
                // Se selecionou um ator, não iniciar arraste
                _isDragging = false;
                _dragButton = -1;
                return true;
            }
        }
        
        // Inicia o arraste
        _isDragging = true;
        _dragButton = button;
        
        // Obtém posição atual do cursor (método herdado de GLWindow)
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

    return true; // Retorna true para impedir que a classe base processe e cause crash
}

bool MainWindow::mouseMoveEvent(double xPos, double yPos)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    if (!_isDragging) return false;

    auto cam = _renderer ? _renderer->camera() : nullptr;
    if (!cam) return false;

    // Calcula delta do movimento
    float dx = (float)(xPos - _lastX);
    float dy = (float)(yPos - _lastY);

    // Atualiza última posição
    _lastX = xPos;
    _lastY = yPos;

    if (dx == 0 && dy == 0) return true;

    // Botão Direito: Orbit (Girar câmera)
    if (_dragButton == GLFW_MOUSE_BUTTON_RIGHT)
    {
        const float sensitivity = 0.5f;
        cam->azimuth(-dx * sensitivity);   // Gira no eixo Y
        cam->elevation(-dy * sensitivity); // Gira no eixo X
    }
    // Botão Esquerdo ou Meio: Pan (Mover câmera lateralmente)
    else if (_dragButton == GLFW_MOUSE_BUTTON_LEFT || _dragButton == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        // Ajusta velocidade do pan baseado na distância para parecer natural
        float panSpeed = cam->distance() * 0.002f;
        cam->translate(-dx * panSpeed, dy * panSpeed, 0.0f);
    }

    return true;
}

bool MainWindow::scrollEvent(double xOffset, double yOffset)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    auto cam = _renderer ? _renderer->camera() : nullptr;
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
    // Selecionar ator apenas no modo OpenGL (não durante ray casting de imagem)
    if (ImGui::GetIO().WantCaptureMouse) return false;
    
    if (_rayCaster == nullptr) return false;
    
    // Inverter coordenada Y (OpenGL tem origem no canto inferior esquerdo)
    int glY = height() - y;
    
    // Selecionar ator através de ray casting
    _selectedActor = _rayCaster->selectActor(x, glY);
    
    // Atualizar GUI com ator selecionado
    if (_gui && _selectedActor)
    {
        // Encontrar índice do ator selecionado
        auto scene = _scene;
        if (scene)
        {
            const auto& actors = scene->actors();
            for (size_t i = 0; i < actors.size(); ++i)
            {
                if (actors[i] == _selectedActor)
                {
                    // Atualizar seleção na GUI (precisamos expor isso)
                    // Por enquanto, apenas log
                    printf("Selected actor: %s\n", _selectedActor->name());
                    break;
                }
            }
        }
    }
    
    return _selectedActor != nullptr;
}

void
MainWindow::render()
{
  if (_isMinimized) return;
  
  if (_renderer != nullptr)
    _renderer->render();
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