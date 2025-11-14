#include "MainWindow.h"
#include <imgui.h>
#include <GL/gl3w.h> 

using namespace std;
using namespace cg; 

MainWindow::MainWindow(int width, int height)
    : cg::GLRenderWindow3("P1", width, height), // CORRIGIDO: Chama a base
      _scene(new Scene{}),
      _mode(RenderMode::RayCasting),
      _sphereColor(Color::red),
      _planeColor(Color::gray),
      _sphereMatA(nullptr),
      _sphereMatS(nullptr),
      _sphereMatSh(nullptr),
      _planeMat(nullptr),
      _selectedActor(nullptr)
{
    //A Base::initialize() agora cria a câmera, então a classe MainWindow
    //não precisa ter o membro '_camera' declarado no .h
}

void MainWindow::initialize()
{
    //Chama a inicialização da base (cria o contexto GL)
    Base::initialize();

    //Obtém a câmera da classe base
    auto cam = camera();

    //Configuração de Câmera estável
    vec3f eye{ 0, 5, 15 };
    vec3f target{ 0, 1, 0 }; //Alvo no centro da cena
    vec3f up{ 0, 1, 0 }; 
    vec3f dir = (target - eye).versor();

    cam->setPosition(eye);
    cam->setDirectionOfProjection(dir);
    cam->setViewUp(up);
    cam->setViewAngle(45.0f);
    cam->setAspectRatio(float(width()) / float(height()));
    cam->setNearPlane(0.1f);

    createScene();
    rayCasting(); //Renderiza o primeiro quadro 
}


bool MainWindow::keyInputEvent(int key, int action, int mods)
{
    if (Base::keyInputEvent(key, action, mods))
        return true;

    if (action != GLFW_RELEASE && mods == GLFW_MOD_ALT)
    {
        if (key == GLFW_KEY_P)
        {
            _mode = (_mode == RenderMode::OpenGL)
                        ? RenderMode::RayCasting
                        : RenderMode::OpenGL;
            return true;
        }
    }

    return false;
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

bool MainWindow::onMouseLeftPress(int x, int y)
{
    auto cam = camera();
    vec3 camPos = cam->position();

    const float nearP = cam->nearPlane();
    const float viewH = cam->windowHeight();
    const float viewW = viewH * cam->aspectRatio();

    float ndcX = float(x) / float(width());
    float ndcY = 1.0f - float(y) / float(height());

    float x_cam = (ndcX - 0.5f) * viewW;
    float y_cam = (ndcY - 0.5f) * viewH;

    vec3 pCamera{ x_cam, y_cam, -nearP };
    vec3 pWorld = cam->cameraToWorld(pCamera);

    vec3 dir = (pWorld - camPos).versor();
    Ray3f ray(camPos, dir);

    ::Intersection hit;
    if (_scene->intersect(ray, hit))
    {
        _selectedActor = (Actor*)hit.actor;
        printf("Ator selecionado!\n");
    }
    else
    {
        _selectedActor = nullptr;
        printf("Nenhum ator atingido.\n");
    }

    return true;
}

void MainWindow::gui()
{
    ImGui::SetNextWindowSize({360, 220});
    ImGui::Begin("P1 GUI");

    bool changed = false;

    if (ImGui::ColorEdit3("Spheres Color", (float*)&_sphereColor))
    {
        _sphereMatA->diffuse = _sphereColor;
        _sphereMatS->diffuse = _sphereColor;
        _sphereMatSh->diffuse = _sphereColor;
        changed = true;
    }

    if (ImGui::ColorEdit3("Plane Color", (float*)&_planeColor))
    {
        _planeMat->diffuse = _planeColor;
        changed = true;
    }

    ImGui::Separator();

    ImGui::Text("Render Mode (Alt + P):");
    if (ImGui::RadioButton("OpenGL", _mode == RenderMode::OpenGL))
        _mode = RenderMode::OpenGL;
    ImGui::SameLine();
    if (ImGui::RadioButton("Ray Casting", _mode == RenderMode::RayCasting))
        _mode = RenderMode::RayCasting;

    if (_mode == RenderMode::RayCasting)
    {
        if (camera()->modified())
            rayCasting();

        if (ImGui::Button("Re-render"))
            rayCasting();
    }

    ImGui::Separator();
    ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", deltaTime(), ImGui::GetIO().Framerate);
    ImGui::End();

    if (_selectedActor)
    {
        ImGui::Begin("Inspector");

        Material* mat = _selectedActor->material();
        if (mat)
        {
            bool edited = false;

            if (ImGui::ColorEdit3("Base Color", (float*)&mat->diffuse))
                edited = true;
            if (ImGui::SliderFloat("Metallic", &mat->specular.r, 0.f, 1.f))
                edited = true;
            if (ImGui::SliderFloat("Roughness", &mat->shine, 0.f, 1.f))
                edited = true;

            if (edited && _mode == RenderMode::RayCasting)
                rayCasting();
        }

        ImGui::End();
    }

    if (changed && _mode == RenderMode::RayCasting)
        rayCasting();
}

void MainWindow::createScene()
{
    _sphereMatA = new Material(_sphereColor);
    _sphereMatA->ambient = Color(1,1,1);
    _sphereMatA->specular = Color(1,1,1);
    _sphereMatA->shine = 32;

    _sphereMatS = new Material(_sphereColor);
    _sphereMatS->ambient = Color(0.05f,0.05f,0.05f);
    _sphereMatS->specular = Color(0.5f,0.5f,0.5f);
    _sphereMatS->shine = 32;

    _sphereMatSh = new Material(_sphereColor);
    _sphereMatSh->ambient = Color(0.05f,0.05f,0.05f);
    _sphereMatSh->specular = Color(1,1,1);
    _sphereMatSh->shine = 64;

    _planeMat = new Material(_planeColor);
    _planeMat->ambient = Color(0.1f,0.1f,0.1f);
    _planeMat->specular = Color(0.5f,0.5f,0.5f);
    _planeMat->shine = 64;

    auto sphereA = new Sphere();
    auto sphereS = new Sphere();
    auto sphereSh = new Sphere();
    auto plane = new Plane();

    auto actorA = new Actor(sphereA, _sphereMatA);
    auto actorS = new Actor(sphereS, _sphereMatS);
    auto actorSh = new Actor(sphereSh, _sphereMatSh);
    auto actorPlane = new Actor(plane, _planeMat);

    _scene->addActor(actorA);
    _scene->addActor(actorS);
    _scene->addActor(actorSh);
    _scene->addActor(actorPlane);

    using Mat4 = Matrix<float,4,4>;

    auto T = [](float x,float y,float z){
        Mat4 m; m.identity();
        m(0,3)=x; m(1,3)=y; m(2,3)=z;
        return m;
    };

    auto S = [](float x,float y,float z){
        Mat4 m; m.identity();
        m(0,0)=x; m(1,1)=y; m(2,2)=z;
        return m;
    };

    actorA->setTransform(T(2,1,0));
    actorS->setTransform(T(-2,1,2));
    actorSh->setTransform(T(-2,1,-2));
    actorPlane->setTransform(S(10,1,10));

    auto box = new Actor(new Box(), _sphereMatS);
    box->setTransform(T(0,1,0));
    _scene->addActor(box);

    auto L1 = new Light();
    L1->setType(Light::Type::Point);
    L1->color = Color(1,1,1);
    L1->setPosition({0,2,0});
    _scene->addLight(L1);

    auto L2 = new Light();
    L2->setType(Light::Type::Point);
    L2->color = Color(0.5f,0.01f,0.01f);
    L2->setPosition({2.5f,2,0});
    _scene->addLight(L2);

    auto L3 = new Light();
    L3->setType(Light::Type::Point);
    L3->color = Color(0.01f,0.01f,0.5f);
    L3->setPosition({0,2,2.5f});
    _scene->addLight(L3);

    _scene->background = Color(0.05f,0.05f,0.05f);
    _scene->ambientLight = Color(0.2f,0.2f,0.2f);

    _scene->buildBVH();
}

void MainWindow::rayCasting()
{
    int w = width();
    int h = height();

    if (!_image || _image->width() != w || _image->height() != h)
        _image = new GLImage(w, h);

    _scene->render(*camera(), *_image);
}

void MainWindow::renderScene()
{
    if (_mode == RenderMode::RayCasting)
    {
        if (_image)
            _image->draw(0,0);
        return;
    }
    
    // Modo OpenGL: O código de desenho está desativado.
}