#include "MainWindow.h"
#include "PBRActor.h"
#include "imgui.h"
#include "GUIInitializer.h"

namespace cg
{

void
MainWindow::initialize()
{
    GLRenderWindow3::initialize();

    _scene = SceneBuilder::buildDefaultScene();

    auto camera = this->camera();

    // Configuração inicial da câmera: Orbitando a origem (0,0,0).
    vec3f initialPos{0, 0, 15};

    camera->setTransform(initialPos, quatf::identity());
    camera->setDistance(15.0f);
    camera->setClippingPlanes(0.1f, 100.0f);
    camera->setProjectionType(Camera::Perspective);
    camera->setViewAngle(45.0f);
    camera->setEulerAngles({0, 0, 0});
    camera->setAspectRatio((float)width() / (float)height());

    // Inicialização do pipeline de Rasterização (OpenGL).
    _renderer = new PBRRenderer{*_scene, *camera};
    _renderer->setImageSize(width(), height());

    // Inicialização do pipeline de Ray Casting (CPU) para seleção e renderização alternativa.
    _rayCaster = new RayCaster{*_scene, *camera};
    _rayCaster->setImageSize(width(), height());

    // Configuração de estado global do OpenGL.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    printf("MainWindow initialized\n");
    printf("Scene: %d actors, %d lights\n", 
            _scene->actorCount(), 
            _scene->lightCount());
}

void MainWindow::update()
{
    camera()->update();
}

void MainWindow::resetScene()
{
  // Preserva o estado atual da câmera (transformação) antes de recriar a cena.
  auto oldCam = _renderer->camera();
  vec3f pos = oldCam->position();
  vec3f angles = oldCam->eulerAngles();

  // Liberação de memória das estruturas antigas.
  delete _renderer;
  delete _scene;
  
  _scene = SceneBuilder::buildDefaultScene();
  
  // Reinstanciação da câmera com parâmetros preservados.
  auto camera = new Camera;
  camera->setPosition(pos);
  camera->setEulerAngles(angles);
  camera->setClippingPlanes(0.1f, 100.0f);
  camera->setProjectionType(Camera::Perspective);
  camera->setViewAngle(45.0f);
  camera->setAspectRatio((float)width() / (float)height());

  _renderer = new PBRRenderer{*_scene, *camera};
  _renderer->setImageSize(width(), height());
  
  // Reconstrução do RayCaster e BVH.
  delete _rayCaster;
  _rayCaster = new RayCaster{*_scene, *camera};
  _rayCaster->setImageSize(width(), height());
  
  if (_enableRayCaster && _rayCaster)
    _rayCaster->rebuildBVH();
}

bool MainWindow::windowResizeEvent(int width, int height)
{
    // Tratamento para minimização da janela.
    if (width == 0 || height == 0)
    {
        _isMinimized = true;
        return true;
    }
    _isMinimized = false;

    // Atualização das dimensões de viewport nos renderizadores.
    if (_renderer)
        _renderer->setImageSize(width, height);
    if (_rayCaster)
        _rayCaster->setImageSize(width, height);

    // Ajuste da razão de aspecto da câmera.
    auto cam = camera();
    if (cam)
        cam->setAspectRatio((float)width / (float)height);

    return GLWindow::windowResizeEvent(width, height);
}

bool MainWindow::keyInputEvent(int key, int action, int mods)
{
    // Delega evento para ImGui se a interface estiver ativa.
    if (ImGui::GetIO().WantCaptureKeyboard) return false;
    
    if (action == GLFW_RELEASE) return false;

    auto cam = camera();
    if (!cam) return false;

    // Movimentação da câmera em espaço local (WASD + QZ).
    const float speed = 0.5f; 
    vec3f d = vec3f::null();

    switch (key)
    {
        case GLFW_KEY_W: d.z -= speed; break; // Forward
        case GLFW_KEY_S: d.z += speed; break; // Backward
        case GLFW_KEY_A: d.x -= speed; break; // Left
        case GLFW_KEY_D: d.x += speed; break; // Right
        case GLFW_KEY_Q: d.y += speed; break; // Up
        case GLFW_KEY_Z: d.y -= speed; break; // Down
        default: return false;
    }

    cam->translate(d);
    return true;
}

bool MainWindow::mouseButtonInputEvent(int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    // Botão Esquerdo: Seleção (Press) ou Pan (Drag).
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // Tenta realizar Picking via classe base (invoca onMouseLeftPress).
        bool handled = GLRenderWindow3::mouseButtonInputEvent(button, action, mods);
        
        // Se não houve seleção (Picking), inicia lógica de arrasto (Pan).
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
    
    // Outros botões (Direito/Meio) iniciam lógica de arrasto imediatamente.
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

    return GLRenderWindow3::mouseButtonInputEvent(button, action, mods);
}

bool MainWindow::mouseMoveEvent(double xPos, double yPos)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;

    if (_isDragging)
    {
        auto cam = camera();
        if (!cam) return false;

        float dx = (float)(xPos - _lastX);
        float dy = (float)(yPos - _lastY);

        _lastX = xPos;
        _lastY = yPos;

        if (dx == 0 && dy == 0) return true;

        // Botão Esquerdo ou Meio: Pan (Translação no plano da câmera).
        if (_dragButton == GLFW_MOUSE_BUTTON_LEFT || _dragButton == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            float panSpeed = cam->distance() * 0.002f;
            cam->translate(-dx * panSpeed, dy * panSpeed, 0.0f);
        }
        // Botão Direito: Orbit (Rotação em torno do ponto focal).
        else if (_dragButton == GLFW_MOUSE_BUTTON_RIGHT)
        {
            // Assegura rotação em torno da origem (0,0,0) corrigindo drift do ponto focal.
            vec3f origin{0, 0, 0};
            vec3f currentFocal = cam->focalPoint();
            float distToOrigin = (currentFocal - origin).length();
            
            if (distToOrigin > 0.1f)
            {
                float currentDist = cam->distance();
                vec3f direction = cam->directionOfProjection();
                vec3f newPos = origin - direction * currentDist;
                cam->setPosition(newPos);
            }
            
            const float sensitivity = 0.5f;
            // rotateYX com flag orbit=true.
            cam->rotateYX(-dx * sensitivity, -dy * sensitivity, true);
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
        float zoomFactor = (yOffset > 0) ? 1.1f : 0.9f;
        cam->zoom(zoomFactor);
    }
    return true;
}

bool MainWindow::onMouseLeftPress(int x, int y)
{
    if (ImGui::GetIO().WantCaptureMouse) return false;
    
    if (_rayCaster == nullptr) 
    {
        printf("Error: RayCaster not initialized.\n");
        return false;
    }
    
    // Conversão de coordenadas: Sistema de Janela (Top-Left) para OpenGL (Bottom-Left).
    int glY = height() - y;
    
    // Executa Ray Picking usando a estrutura de aceleração (BVH) do RayCaster.
    _selectedActor = _rayCaster->selectActor(x, glY);
    
    // Sincroniza a seleção com o renderizador OpenGL para feedback visual (e.g., Bounding Box).
    if (_renderer)
        _renderer->setSelectedActor(_selectedActor);
    
    if (_selectedActor)
    {
        printf("Selected Actor: %s\n", _selectedActor->name());
        return true;
    }
    
    return false;
}

Camera* MainWindow::getCamera()
{
    return camera();
}

void MainWindow::render()
{
    if (_isMinimized) return;

    // Pipeline do Ray Caster.
    if (_enableRayCaster)
    {
        if (_rayCaster)
        {
            auto camera = this->camera();
            if (!camera) return;

            bool imageInvalid = (!_image || _image->width() != width() || _image->height() != height());
            uint32_t currentStamp = camera->timestamp();
            static uint32_t lastCameraTimestamp = 0; 
            bool cameraChanged = (currentStamp != lastCameraTimestamp);

            if (imageInvalid || cameraChanged)
            {
                if (imageInvalid)
                {
                    if (_image) delete _image; 
                    _image = new GLImage(width(), height());
                    _rayCaster->setImageSize(width(), height());
                }
                _rayCaster->renderImage(camera, _image);
                lastCameraTimestamp = currentStamp;
            }
            
            // Exibe o buffer de imagem gerado como uma textura OpenGL.
            if (_image)
                _image->draw(0, 0);
        }
    }
    else
    {
        // Pipeline de Rasterização padrão (OpenGL).
        if (_renderer != nullptr)
            _renderer->render();
    }
}

void MainWindow::gui()
{
    if (_gui) _gui->draw();
}

void MainWindow::terminate()
{
    delete _renderer;
    delete _scene;
    printf("MainWindow terminated\n");
}

}