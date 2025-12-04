#pragma once

#include "Scene.h"
#include "graphics/Light.h"
#include "PBRActor.h"
#include "Sphere.h"
#include "Plane.h"
#include "Box.h"
#include <cstring>

namespace cg
{

// Classe utilitária estática para construção e população da cena padrão.
class SceneBuilder
{
public:
  // Inicializa a cena principal com iluminação e geometria de teste PBR.
  static Scene* buildDefaultScene()
  {
    auto scene = new Scene{"TP1 PBR Scene"};
    
    scene->backgroundColor = Color::gray;
    
    addLights(scene);
    addFloor(scene);
    addActors(scene);
    
    return scene;
  }

private:
  enum ShapeType
  {
    BOX,
    SPHERE
  };

  static constexpr float boxDimension = 2.0f; // Configurável (Deixei em 2 por padrão)
  static constexpr float sphereRadius = boxDimension / 2.0f;

  // Configura um sistema de iluminação de três pontos (Key, Fill, Back lights).
  static void addLights(Scene* scene)
  {
    // Luz Principal (Key Light) - Branca constante
    {
      auto light = new Light{};
      light->setName("Light 1");
      light->setType(Light::Type::Point);
      light->setPosition({0, 10, 0});
      light->color = Color::white;
      light->falloff = Light::Falloff::Constant;
      scene->addLight(light);
    }
    
    // Luz de Preenchimento (Fill Light) - Vermelho linear
    {
      auto light = new Light{};
      light->setName("Light 2");
      light->setType(Light::Type::Point);
      light->setPosition({10, 10, 5});
      light->color = Color{1.0f, 0.0f, 0.0f};
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
    
    // Luz de Recorte (Back Light) - Azul linear
    {
      auto light = new Light{};
      light->setName("Light 3");
      light->setType(Light::Type::Point);
      light->setPosition({-10, 10, -5});
      light->color = Color{0.0f, 0.0f, 1.0f}; 
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
  }

  static void addFloor(Scene* scene)
  {
    auto shape = new Plane{50.0f, 25.0f};
    
    auto material = new PBRMaterial(
        Color{0.2f, 1.0f, 0.9f},
        Color{1.0f, 1.0f, 1.0f},
        0.4f,
        0.1f
    );

    auto actor = new PBRActor{"Floor", shape, material};
    actor->setPosition({0.0f, -0.01f, 0.0f});

    if (shape && shape->mesh())
    {
        auto mesh = const_cast<TriangleMesh*>(shape->mesh());
        if (mesh)
            std::memset(&mesh->userData, 0, sizeof(mesh->userData));
    }

    scene->addActor(actor);
  }
  
  // Organiza os atores em fileiras para demonstração de materiais.
  static void addActors(Scene* scene)
  {
    const float xSpacing = 2.5f;
    const float zSpacing = 3.0f;
    
    const float actorY = sphereRadius + 0.01f;

    int count = 12;

    float startX = float(-count);

    // Esferas Dielétricas (Colors)
    addActorRow(scene, ShapeType::SPHERE, false, {startX, actorY, -zSpacing * 1.5f}, xSpacing, count);
    
    // Boxes Dielétricos
    addActorRow(scene, ShapeType::BOX, false, {startX, actorY, -zSpacing * 0.5f}, xSpacing, count);
    
    // Esferas Metálicas (Presets)
    addActorRow(scene, ShapeType::SPHERE, true, {startX, actorY, zSpacing * 0.5f}, xSpacing, count);

    // Boxes Metálicos (Presets)
    addActorRow(scene, ShapeType::BOX, true, {startX, actorY, zSpacing * 1.5f}, xSpacing, count);
  }

  static void addActorRow(Scene* scene, ShapeType type, bool metal, const vec3f& startPos, float spacing, int count)
  {
    static const std::vector<Color> dielectricColors = {
      Color{0.8f, 0.2f, 0.2f}, Color{0.2f, 0.8f, 0.2f}, Color{0.2f, 0.2f, 0.8f},
      Color{0.8f, 0.8f, 0.2f}, Color{0.8f, 0.2f, 0.8f}, Color{0.2f, 0.8f, 0.8f}
    };

    static const std::vector<std::string> metalNames = { "Copper", "Aluminum", "Silver", "Titanium", "Gold" };

    using MetalFactoryFunc = PBRMaterial*(*)(float);
    static const MetalFactoryFunc metalFactories[] = {
        &PBRMaterial::copper, &PBRMaterial::aluminum,
        &PBRMaterial::silver, &PBRMaterial::titanium,
        &PBRMaterial::gold
    };

    std::function<Shape3*()> createShape;
    std::string shapeNamePrefix;

    if (type == ShapeType::BOX)
    {
      shapeNamePrefix = "Box";
      createShape = []() { return new Box{boxDimension}; };
    }
    else // SPHERE
    {
      shapeNamePrefix = "Sphere";
      createShape = []() { return new Sphere{sphereRadius, 3}; };
    }

    std::function<PBRMaterial*(int, float)> createMaterial;
    std::function<std::string(int)> getNameSuffix;

    if (metal) // Metais
    {
      createMaterial = [](int i, float r) {
        return metalFactories[i % 5](r); 
      };
      getNameSuffix = [](int i) { return metalNames[i % 5]; };
    }
    else // Dielétricos
    {
      createMaterial = [](int i, float r) {
        return PBRMaterial::dielectric(dielectricColors[i % dielectricColors.size()], r);
      };
      getNameSuffix = [](int i) { return "Dielectric"; };
    }

    for (int i = 0; i < count; ++i)
    {
      float t = (count > 1) ? (float)i / (count - 1) : 0.5f;
      float roughness = 0.1f + (0.8f * t);

      Shape3* shape = createShape();
      PBRMaterial* material = createMaterial(i, roughness);
      
      // Formatação de nome
      char name[64];
      std::string suffix = getNameSuffix(i);
      snprintf(name, sizeof(name), "%s_%s_%d", shapeNamePrefix.c_str(), suffix.c_str(), i);

      vec3f position = startPos + vec3f{i * spacing, 0, 0};

      auto actor = new PBRActor{name, shape, material};
      actor->setPosition(position);
      
      if (shape && shape->mesh())
      {
          auto mesh = const_cast<TriangleMesh*>(shape->mesh());
          if (mesh)
              std::memset(&mesh->userData, 0, sizeof(mesh->userData));
      }

      scene->addActor(actor);
    }
  }
};

}