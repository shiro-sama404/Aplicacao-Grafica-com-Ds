#pragma once

#include "graphics/GLRenderWindow3.h"
#include <graphics/GLImage.h>
#include <graphics/Camera.h>
#include "Plane.h"
#include "Scene.h"
#include "Sphere.h"

class MainWindow final : public cg::GLRenderWindow3
{
public:
    MainWindow(int width, int height);

    ~MainWindow() override
    {}

private:
    using Base = cg::GLRenderWindow3;

    // --- Membros de estado da GUI ---
    cg::Color _sphereColor;
    cg::Color _planeColor;
    bool _enableRayCasting;

    // --- Ponteiros de Material (para atualiza��o via GUI) ---
    cg::Material* _sphereMatA;
    cg::Material* _sphereMatS;
    cg::Material* _sphereMatSh;
    cg::Material* _planeMat;

    // --- Objetos Principais da Cena ---
    Reference<Scene> _scene;
    Reference<GLImage> _image;
    Reference<Camera> _camera;

    // --- M�todos Sobrescritos do GLRenderWindow3 ---
    void initialize() override;
    void update() override;
    void renderScene() override;
    bool keyInputEvent(int, int, int) override;
    bool mouseButtonInputEvent(int button, int action, int mods) override;
    bool scrollEvent(double xoffset, double yoffset) override;
    bool mouseMoveEvent(double xPos, double yPos) override;
    void gui() override;

    // --- M�todos de L�gica Interna ---

    void rayCasting();
    void createScene();
};