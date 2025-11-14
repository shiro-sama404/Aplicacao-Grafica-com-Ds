#include "MainWindow.h"

using namespace std;

MainWindow::MainWindow(int width, int height) :
    Base{ "P1", width, height },
    _enableRayCasting{ true },
    _scene{ new Scene{} },
    _camera{ {} },
    _image{ nullptr },
    _sphereColor{ cg::Color::red },
    _planeColor{ cg::Color::gray },
    _sphereMatA{ nullptr },
    _sphereMatS{ nullptr },
    _sphereMatSh{ nullptr },
    _planeMat{ nullptr }
{}

void
MainWindow::initialize()
{
    Base::initialize();
    _camera = camera();

    // Configuração inicial da câmera
    vec3f eye{ 5, 2, 12 };
    vec3f target{ 0, -1, 0 };
    vec3f up{ 0, -1, 0 };
    vec3f dir = (target - eye).versor();

    _camera->setPosition(eye);
    _camera->setDirectionOfProjection(dir);
    _camera->setViewUp(up);
    _camera->setViewAngle(45.0f);
    _camera->setAspectRatio(float(this->width()) / float(this->height()));
    _camera->setNearPlane(0.1f);
    _camera->setProjectionType(Camera::Perspective);

    createScene();

    if (!_image)
        rayCasting();
}

void
MainWindow::update()
{
    // Refaz o ray casting se a câmera for modificada
    if (_enableRayCasting && _camera->modified())
        rayCasting();
    _camera->update();
}

bool
MainWindow::keyInputEvent(int key, int action, int mods)
{
    // ALT + P para ligar/desligar o ray casting
    if (action != GLFW_RELEASE && mods == GLFW_MOD_ALT)
        switch (key)
        {
        case GLFW_KEY_P:
            _enableRayCasting ^= true;
            return true;
        }

    return Base::keyInputEvent(key, action, mods);
}

bool MainWindow::mouseButtonInputEvent(int button, int action, int mods)
{
    return Base::mouseButtonInputEvent(button, action, mods);
}

bool MainWindow::scrollEvent(double xoffset, double yoffset)
{
    return Base::scrollEvent(xoffset, yoffset);
}

bool MainWindow::mouseMoveEvent(double xPos, double yPos)
{
    return Base::mouseMoveEvent(xPos, yPos);
}

void
MainWindow::gui()
{
    ImGui::SetNextWindowSize({ 360, 180 });
    ImGui::Begin("P1 GUI");

    bool colorChanged = false; // Flag pra mudança de cor

    // Editor de cor para as esferas
    if (ImGui::ColorEdit3("Spheres Color", (float*)&_sphereColor))
    {
        // Atualiza a cor difusa dos materiais
        _sphereMatA->diffuse = _sphereColor;
        _sphereMatS->diffuse = _sphereColor;
        _sphereMatSh->diffuse = _sphereColor;
        colorChanged = true;
    }

    // Editor de cor para o plano
    if (ImGui::ColorEdit3("Plane Color", (float*)&_planeColor))
    {
        _planeMat->diffuse = _planeColor;
        colorChanged = true;
    }

    ImGui::Separator();
    ImGui::Checkbox("Enable Ray Casting", &_enableRayCasting);
    ImGui::Separator();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        deltaTime(),
        ImGui::GetIO().Framerate);
    ImGui::End();

    // Renderiza se a cor mudou
    if (colorChanged && _enableRayCasting)
    {
        rayCasting();
    }
}

void MainWindow::createScene()
{
    // === Materiais ===
    _sphereMatA = new Material(_sphereColor);
    _sphereMatA->ambient = Color(1.0f, 1.0f, 1.0f);
    _sphereMatA->specular = Color(1.0f, 1.0f, 1.0f);
    _sphereMatA->shine = 32.0f;

    _sphereMatS = new Material(_sphereColor);
    _sphereMatS->ambient = Color(0.05f, 0.05f, 0.05f);
    _sphereMatS->specular = Color(0.5f, 0.5f, 0.5f);
    _sphereMatS->shine = 32.0f;

    _sphereMatSh = new Material(_sphereColor);
    _sphereMatSh->ambient = Color(0.05f, 0.05f, 0.05f);
    _sphereMatSh->specular = Color(1.0f, 1.0f, 1.0f);
    _sphereMatSh->shine = 64.0f;

    _planeMat = new Material(_planeColor);
    _planeMat->ambient = Color(0.1f, 0.1f, 0.1f);
    _planeMat->specular = Color(0.5f, 0.5f, 0.5f);
    _planeMat->shine = 64;

    // === Formas ===
    auto sphereA = new Sphere(vec3f{ 2, 1, 0 }, 1.0f);
    auto sphereS = new Sphere(vec3f{ -2, 1, 2 }, 1.0f);
    auto sphereSh = new Sphere(vec3f{ -2, 1, -2 }, 1.0f);
    auto plane = new Plane(vec3f{ 0, 1, 0 }, 0.0f);

    // === Atores ===
    _scene->addActor(new Actor(sphereA, _sphereMatA));
    _scene->addActor(new Actor(sphereS, _sphereMatS));
    _scene->addActor(new Actor(sphereSh, _sphereMatSh));
    _scene->addActor(new Actor(plane, _planeMat));

    // === Luzes ===
    vec3f lightPos1 = vec3f{ 0, 2.0f, 0 };
    vec3f lightPos2 = vec3f{ 2.5f, 2.0f, 0 };
    vec3f lightPos3 = vec3f{ 0, 2.0f, 2.5f };

    auto light1 = new Light();
    light1->setType(Light::Type::Point);
    light1->color = Color{ 1.0f, 1.0f, 1.0f };
    light1->setPosition(lightPos1);
    _scene->addLight(light1);

    auto light2 = new Light();
    light2->setType(Light::Type::Point);
    light2->color = Color{ 0.5f, 0.01f, 0.01f };
    light2->setPosition(lightPos2);
    _scene->addLight(light2);

    auto light3 = new Light();
    light3->setType(Light::Type::Point);
    light3->color = Color{ 0.01f, 0.01f, 0.5f };
    light3->setPosition(lightPos3);
    _scene->addLight(light3);


    // === Configurações da Cena ===
    _scene->background = Color(0.05f, 0.05f, 0.05f);
    _scene->ambientLight = Color{ 0.2f, 0.2f, 0.2f };
}

void MainWindow::rayCasting()
{
    int w = width();
    int h = height();

    // Recria a imagem se a janela for redimensionada
    if (!_image || _image->width() != w || _image->height() != h)
        _image = new GLImage(w, h);

    // Executa o render da cena na imagem
    _scene->render(*_camera, *_image);
}

void
MainWindow::renderScene()
{
    if (_enableRayCasting && _image)
        _image->draw(0, 0);
}