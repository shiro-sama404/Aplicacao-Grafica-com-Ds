#pragma once

#include "Scene.h"
#include "graphics/Light.h"
#include "PBRActor.h"
#include "Sphere.h"
#include "Plane.h"
#include "Cube.h"
#include "Box.h"

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
  // Configura um sistema de iluminação de três pontos (Key, Fill, Back lights).
  static void addLights(Scene* scene)
  {
    // Luz Principal (Key Light) - Branca constante
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({5, 10, -20});
      light->color = Color::white;
      light->falloff = Light::Falloff::Constant;
      scene->addLight(light);
    }
    
    // Luz de Preenchimento (Fill Light) - Vermelho linear
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({10, 10, 0});
      light->color = Color{1.0f, 0.0f, 0.0f};
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
    
    // Luz de Recorte (Back Light) - Azul linear
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({-10, 20, 10});
      light->color = Color{0.0f, 0.0f, 1.0f}; 
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
  }

  // Adiciona o plano de chão com rotação adequada e material base.
  static void addFloor(Scene* scene)
  {
    auto shape = new Plane{25.0f, 25.0f};
    
    auto material = new PBRMaterial(
        Color{0.2f, 0.2f, 0.2f}, 
        Color{0.1f, 0.1f, 0.1f}, 
        0.6f,                    
        0.0f                     
    );

    auto actor = new PBRActor{"Floor", shape, material};
    scene->addActor(actor);
  }
  
  // Organiza os atores em fileiras para demonstração de materiais.
  static void addActors(Scene* scene)
  {
    const float xSpacing = 2.5f; 
    const float zSpacing = 3.0f; 
    float startX = -5.0f;

    float sphereY = 1.0f;
    float boxY = 1.0f;

    // Esferas Dielétricas
    addDielectricRow(scene, {startX, sphereY, -zSpacing * 1.5f}, xSpacing);
    
    // Boxes com propriedades mistas
    addBoxRow(scene, {startX, boxY, -zSpacing * 0.5f}, xSpacing);
    
    // Esferas Metálicas
    addMetalRow(scene, {startX, sphereY, zSpacing * 0.5f}, xSpacing);

    // Boxes Metálicos
    addMetalBoxRow(scene, {startX, boxY, zSpacing * 1.5f}, xSpacing);
  }
  
  // Gera fileira de esferas não-metálicas com rugosidade variável.
  static void addDielectricRow(Scene* scene, const vec3f& startPos, float spacing)
  {
    Color colors[] = {
      Color{0.8f, 0.2f, 0.2f}, Color{0.2f, 0.8f, 0.2f}, Color{0.2f, 0.2f, 0.8f},
      Color{0.8f, 0.8f, 0.2f}, Color{0.8f, 0.2f, 0.8f}
    };
    float roughnesses[] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      
      auto material = PBRMaterial::dielectric(colors[i], roughnesses[i]);
      auto shape = new Sphere{1.0f, 3};
      
      char name[32];
      snprintf(name, 32, "Dielectric_%d", i);
      
      auto actor = new PBRActor{name, shape, material};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      scene->addActor(actor);
    }
  }
  
  // Gera fileira de esferas com presets metálicos (Cobre, Alumínio, etc).
  static void addMetalRow(Scene* scene, const vec3f& startPos, float spacing)
  {
    float roughnesses[] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    PBRMaterial* materials[] = {
      PBRMaterial::copper(roughnesses[0]), PBRMaterial::aluminum(roughnesses[1]),
      PBRMaterial::silver(roughnesses[2]), PBRMaterial::titanium(roughnesses[3]),
      PBRMaterial::gold(roughnesses[4])
    };
    const char* names[] = { "Copper", "Aluminum", "Silver", "Titanium", "Gold" };
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      auto shape = new Sphere{1.0f, 3};
      auto actor = new PBRActor{names[i], shape, materials[i]};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      scene->addActor(actor);
    }
  }
  
  // Gera fileira de caixas variando metalicidade e rugosidade.
  static void addBoxRow(Scene* scene, const vec3f& startPos, float spacing)
  {
    Color colors[] = {
      Color{0.9f, 0.1f, 0.1f}, Color{0.1f, 0.9f, 0.1f}, Color{0.1f, 0.1f, 0.9f},
      Color{0.9f, 0.9f, 0.1f}, Color{0.9f, 0.1f, 0.9f}
    };
    float roughnesses[] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
    float metalness[] = {0.0f, 0.2f, 0.5f, 0.8f, 1.0f};
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      
      auto material = new PBRMaterial(
          colors[i], 
          Color{0.04f, 0.04f, 0.04f} * (1.0f - metalness[i]) + colors[i] * metalness[i],
          roughnesses[i],
          metalness[i]
      );
      
      auto shape = new Box{1.5f};
      char name[32];
      snprintf(name, 32, "Box_Mixed_%d", i);
      
      auto actor = new PBRActor{name, shape, material};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      scene->addActor(actor);
    }
  }

  // Gera fileira de caixas puramente metálicas.
  static void addMetalBoxRow(Scene* scene, const vec3f& startPos, float spacing)
  {
    float roughnesses[] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    PBRMaterial* materials[] = {
      PBRMaterial::copper(roughnesses[0]), PBRMaterial::aluminum(roughnesses[1]),
      PBRMaterial::silver(roughnesses[2]), PBRMaterial::titanium(roughnesses[3]),
      PBRMaterial::gold(roughnesses[4])
    };
    const char* names[] = { "Box_Copper", "Box_Aluminum", "Box_Silver", "Box_Titanium", "Box_Gold" };
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      auto shape = new Box{1.5f};
      auto actor = new PBRActor{names[i], shape, materials[i]};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      scene->addActor(actor);
    }
  }

};

}