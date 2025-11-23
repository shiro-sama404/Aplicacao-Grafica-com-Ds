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

namespace cg
{ // begin namespace cg

//
// SceneBuilder: helper para construir a cena do trabalho
//
class SceneBuilder
{
public:
  // Criar cena com configuração padrão do TP1
  // - 10 esferas (5 dielétricos + 5 metais)
  // - 3 luzes pontuais
  // - Disposição em duas fileiras
  static Scene* buildDefaultScene()
  {
    auto scene = new Scene{"TP1 PBR Scene"};
    
    // Configurar cor de fundo
    scene->backgroundColor = Color::black;
    
    // Adicionar luzes pontuais
    addLights(scene);
    
    // Adicionar atores (esferas)
    addActors(scene);
    
    return scene;
  }

private:
  static void addLights(Scene* scene)
  {
    // Luz 1: Frontal superior esquerda (branca)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({-5, 5, 5});
      light->color = Color::white;
      light->falloff = Light::Falloff::Quadratic;
      scene->addLight(light);
    }
    
    // Luz 2: Frontal superior direita (amarelada)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({5, 5, 5});
      light->color = Color{1.0f, 0.95f, 0.8f}; // Luz quente
      light->falloff = Light::Falloff::Quadratic;
      scene->addLight(light);
    }
    
    // Luz 3: Posterior (azulada)
    {
      auto light = new Light{};
      light->setType(Light::Type::Point);
      light->setPosition({0, 3, -8});
      light->color = Color{0.7f, 0.8f, 1.0f}; // Luz fria
      light->falloff = Light::Falloff::Quadratic;
      scene->addLight(light);
    }
  }
  
  static void addActors(Scene* scene)
  {
    const float spacing = 2.5f; // Espaçamento entre esferas
    const float rowOffset = 2.5f; // Distância vertical entre fileiras
    
    // Fileira superior: 5 Dielétricos com rugosidades variadas
    addDielectricRow(scene, {-5.0f, rowOffset, 0}, spacing);
    
    // Fileira inferior: 5 Metais com rugosidades variadas
    addMetalRow(scene, {-5.0f, -rowOffset, 0}, spacing);
  }
  
  static void addDielectricRow(Scene* scene, 
                                const vec3f& startPos,
                                float spacing)
  {
    // Cores para dielétricos
    Color colors[] = {
      Color{0.8f, 0.2f, 0.2f},  // Vermelho
      Color{0.2f, 0.8f, 0.2f},  // Verde
      Color{0.2f, 0.2f, 0.8f},  // Azul
      Color{0.8f, 0.8f, 0.2f},  // Amarelo
      Color{0.8f, 0.2f, 0.8f}   // Magenta
    };
    
    float roughnesses[] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      
      auto material = PBRMaterial::dielectric(colors[i], roughnesses[i]);
      
      // Criar shape (esfera)
      auto shape = new Sphere{1.0f, 3}; // raio=1, subdiv=3
      
      char name[32];
      snprintf(name, 32, "Dielectric_%d", i);
      
      auto actor = new PBRActor{name, shape, material};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      
      scene->addActor(actor);
    }
  }
  
  static void addMetalRow(Scene* scene,
                          const vec3f& startPos,
                          float spacing)
  {
    float roughnesses[] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    
    // Metais: cobre, alumínio, prata, titânio, ouro
    PBRMaterial materials[] = {
      PBRMaterial::copper(roughnesses[0]),
      PBRMaterial::aluminum(roughnesses[1]),
      PBRMaterial::silver(roughnesses[2]),
      PBRMaterial::titanium(roughnesses[3]),
      PBRMaterial::gold(roughnesses[4])
    };
    
    const char* names[] = {
      "Copper", "Aluminum", "Silver", "Titanium", "Gold"
    };
    
    for(int i = 0; i < 5; ++i)
    {
      vec3f position = startPos + vec3f{i * spacing, 0, 0};
      
      // Criar shape (esfera)
      auto shape = new Sphere{1.0f, 3}; // raio=1, subdiv=3
      
      auto actor = new PBRActor{names[i], shape, materials[i]};
      actor->setTransform(mat4f::TRS(position, quatf::identity(), vec3f{1}));
      
      scene->addActor(actor);
    }
  }

}; // SceneBuilder

} // end namespace cg