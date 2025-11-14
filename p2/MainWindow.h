#pragma once

#include "graphics/GLRenderWindow3.h"
#include <graphics/GLImage.h>
#include <graphics/Camera.h>
#include "Plane.h"
#include "Scene.h"
#include "Sphere.h"
#include "Box.h"

class MainWindow final : public cg::GLRenderWindow3
{
public:
    using Base = cg::GLRenderWindow3;     

    MainWindow(int width, int height);
    ~MainWindow() override {}

private:

    //Modo de renderização
    enum RenderMode {
        RayCasting,
        OpenGL
    };
    RenderMode _mode;

    //Cores
    cg::Color _sphereColor;
    cg::Color _planeColor;

    //Materiais
    cg::Material* _sphereMatA;
    cg::Material* _sphereMatS;
    cg::Material* _sphereMatSh;
    cg::Material* _planeMat;

    //Seletor de objeto
    Actor* _selectedActor;

    //Cena e framebuffer
    Reference<Scene> _scene;
    Reference<GLImage> _image;

    //Métodos herdados
    void initialize() override;
    void renderScene() override;

    bool keyInputEvent(int key, int action, int mods) override;
    bool mouseButtonInputEvent(int button, int action, int mods) override;
    bool scrollEvent(double xoffset, double yoffset) override;
    bool mouseMoveEvent(double xPos, double yPos) override;

    bool onMouseLeftPress(int x, int y) override;
    void gui() override;

    //Internos
    void rayCasting();
    void createScene();
};
