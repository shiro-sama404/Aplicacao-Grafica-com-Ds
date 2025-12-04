#pragma once

#include "graphics/Color.h"
#include "graphics/Light.h"
#include "PBRActor.h"
#include <vector>
#include <string>

namespace cg
{

// Classe container responsável pelo gerenciamento de entidades da cena (Atores e Luzes).
class Scene : public SharedObject
{
public:
  Color backgroundColor;

  Scene(const std::string& name = "Scene"):
    _name{name},
    backgroundColor{Color::black}
  {
  }

  const char* name() const
  {
    return _name.c_str();
  }

  void setName(const std::string& name)
  {
    _name = name;
  }

  // --- Gerenciamento de Atores (Geometria) ---

  void addActor(PBRActor* actor)
  {
    if (actor != nullptr)
      _actors.push_back(actor);
  }

  void removeActor(PBRActor* actor)
  {
    auto it = std::find_if(_actors.begin(), _actors.end(), 
      [actor](const Reference<PBRActor>& ref) {
        return ref.get() == actor; 
      });

    if (it != _actors.end())
      _actors.erase(it);
  }

  int actorCount() const
  {
    return (int)_actors.size();
  }

  const std::vector<Reference<PBRActor>>& actors() const
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

  // --- Gerenciamento de Iluminação ---

  void addLight(Light* light)
  {
    if (light != nullptr)
      _lights.push_back(light);
  }

  void removeLight(Light* light)
  {
    auto it = std::find_if(_lights.begin(), _lights.end(), 
      [light](const Reference<Light>& ref) {
        return ref.get() == light; 
      });

    if (it != _lights.end())
      _lights.erase(it);
  }

  int lightCount() const
  {
    return (int)_lights.size();
  }

  const std::vector<Reference<Light>>& lights() const
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

  // Reseta a cena.
  void clear()
  {
    _actors.clear();
    _lights.clear();
  }

private:
  std::string _name;
  std::vector<Reference<PBRActor>> _actors;
  std::vector<Reference<Light>> _lights;
};

}