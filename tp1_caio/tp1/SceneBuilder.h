//[]---------------------------------------------------------------[]
//|                                                                 |
//| SceneBuilder.h                                                  |
//|                                                                 |
//| Scene builder helper for TP1                                   |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Scene.h"
#include "graphics/Light.h"
#include "PBRActor.h"
#include "Sphere.h"
#include "Plane.h"
#include "Cube.h"
#include "Box.h"

namespace cg
{ // begin namespace cg

//
// SceneBuilder: helper para construir a cena do trabalho
//
class SceneBuilder
{
public:
  static Scene* buildDefaultScene()
  {
    auto scene = new Scene{"TP1 PBR Scene"};
    
    // Configurar cor de fundo
    scene->backgroundColor = Color::gray;
    
    // Adicionar luzes
    addLights(scene);
    
    // Adicionar chão e atores
    addFloor(scene);
    addActors(scene);
    
    return scene;
  }

private:
  static void addLights(Scene* scene)
  {
    // Ajustei levemente as luzes para iluminar melhor a área horizontal
    
    // Luz 1: Principal (Branca)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({5, 10, -20});
      light->color = Color::white;
      light->falloff = Light::Falloff::Constant;
      scene->addLight(light);
    }
    
    // Luz 2: Fill Light (Quente)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({10, 10, 0});
      light->color = Color{1.0f, 0.0f, 0.0f};
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
    
    // Luz 3: Back Light (Fria/Azulada)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({-10, 20, 10});
      light->color = Color{0.0f, 0.0f, 1.0f}; 
      light->falloff = Light::Falloff::Linear;
      scene->addLight(light);
    }
  }

  static void addFloor(Scene* scene)
  {
    // Cria plano 25x25 (Originalmente em pé no XY)
    auto shape = new Plane{25.0f, 25.0f};
    
    auto material = new PBRMaterial(
        Color{0.2f, 0.2f, 0.2f}, 
        Color{0.1f, 0.1f, 0.1f}, 
        0.6f,                    
        0.0f                     
    );

    auto actor = new PBRActor{"Floor", shape, material};
    
    // USANDO SEU QUATERNION.H:
    // Rotação de -90 graus no eixo X (vec3f{1,0,0})
    // O construtor é: Quaternion(real angle, const vec3& axis)
    quatf rotation{-90.0f, vec3f{1.0f, 0.0f, 0.0f}};
    
    actor->setTransform(mat4f::TRS({0, 0, 0}, rotation, vec3f{1}));
    
    scene->addActor(actor);
  }
  
  static void addActors(Scene* scene)
  {
    const float xSpacing = 2.5f; 
    const float zSpacing = 3.0f; 
    
    // Posição X inicial para centralizar as fileiras
    float startX = -5.0f;

    // --- CÁLCULO DAS ELEVAÇÕES (Y) ---
    
    // Esfera (Raio 1.0): O centro deve estar em 1.0 para a base tocar o zero.
    // Usamos 1.0f cravado (ou 1.01f se quiser garantir que não haja z-fighting com o chão)
    float sphereY = 1.0f;

    // Box (Tamanho 1.5): A altura total é 1.5, logo a metade é 0.75.
    // O centro deve estar em 0.75 para a base tocar o zero.
    float boxY = 1.0f;

    // --- CRIAÇÃO DAS FILEIRAS ---

    // 1. Fundo: Esferas Dielétricas
    addDielectricRow(scene, {startX, sphereY, -zSpacing * 1.5f}, xSpacing);
    
    // 2. Meio-Fundo: Boxes Mistos
    addBoxRow(scene, {startX, boxY, -zSpacing * 0.5f}, xSpacing);
    
    // 3. Meio-Frente: Esferas Metálicas
    addMetalRow(scene, {startX, sphereY, zSpacing * 0.5f}, xSpacing);

    // 4. Frente: Boxes Metálicos
    addMetalBoxRow(scene, {startX, boxY, zSpacing * 1.5f}, xSpacing);
  }
  
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
      // Alocação segura (Heap)
      auto material = PBRMaterial::dielectric(colors[i], roughnesses[i]);
      auto shape = new Sphere{1.0f, 3};
      
      char name[32];
      snprintf(name, 32, "Dielectric_%d", i);
      
      auto actor = new PBRActor{name, shape, material};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      scene->addActor(actor);
    }
  }
  
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
      // CORREÇÃO: Alocação na Heap
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

}; // SceneBuilder

} // end namespace cg