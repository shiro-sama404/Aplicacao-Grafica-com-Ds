//[]---------------------------------------------------------------[]
//|                                                                 |
//| Scene.h                                                         |
//|                                                                 |
//| Simple scene class for TP1                                     |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "graphics/Color.h"
#include "graphics/Light.h"
#include "PBRActor.h"
#include <vector>
#include <string>

namespace cg
{ // begin namespace cg

//
// Scene: cena simples com atores e luzes
//
class Scene
{
public:
  Color backgroundColor;

  Scene(const std::string& name = "Scene"):
    _name{name},
    backgroundColor{Color::black}
  {
    // do nothing
  }

  ~Scene()
  {
    // Limpar atores
    for (auto actor : _actors)
      delete actor;
    
    // Limpar luzes
    for (auto light : _lights)
      delete light;
  }

  // Nome da cena
  const char* name() const
  {
    return _name.c_str();
  }

  void setName(const std::string& name)
  {
    _name = name;
  }

  // Gerenciamento de atores
  void addActor(PBRActor* actor)
  {
    if (actor != nullptr)
      _actors.push_back(actor);
  }

  void removeActor(PBRActor* actor)
  {
    auto it = std::find(_actors.begin(), _actors.end(), actor);
    if (it != _actors.end())
      _actors.erase(it);
  }

  int actorCount() const
  {
    return (int)_actors.size();
  }

  const std::vector<PBRActor*>& actors() const
  {
    return _actors;
  }

  PBRActor* findActor(const std::string& name) const
  {
    for (auto actor : _actors)
      if (actor->name() == name)
        return actor;
    return nullptr;
  }

  // Gerenciamento de luzes
  void addLight(Light* light)
  {
    if (light != nullptr)
      _lights.push_back(light);
  }

  void removeLight(Light* light)
  {
    auto it = std::find(_lights.begin(), _lights.end(), light);
    if (it != _lights.end())
      _lights.erase(it);
  }

  int lightCount() const
  {
    return (int)_lights.size();
  }

  const std::vector<Light*>& lights() const
  {
    return _lights;
  }

  Light* findLight(const std::string& name) const
  {
    for (auto light : _lights)
      if (light->name() == name)
        return light;
    return nullptr;
  }

  // Limpar cena
  void clear()
  {
    for (auto actor : _actors)
      delete actor;
    _actors.clear();

    for (auto light : _lights)
      delete light;
    _lights.clear();
  }

private:
  std::string _name;
  std::vector<PBRActor*> _actors;
  std::vector<Light*> _lights;

}; // Scene

} // end namespace cg