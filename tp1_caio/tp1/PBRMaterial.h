#pragma once

#include "core/SharedObject.h"
#include "graphics/Color.h"

namespace cg
{

// Estrutura de dados para representação de propriedades de materiais PBR (Physically Based Rendering).
class PBRMaterial: public SharedObject
{
public:
  Color Od;        // Componente de reflexão difusa (Albedo).
  Color Os;        // Componente de reflexão especular (F0 para metais).
  float roughness; // Fator de rugosidade da microfacetas [0, 1].
  float metalness; // Fator de metalicidade [0, 1].

  // Construtor padrão: inicializa como um material dielétrico neutro.
  PBRMaterial():
    Od(Color{0.5f, 0.5f, 0.5f}),
    Os(Color{0.04f, 0.04f, 0.04f}),
    roughness{0.5f},
    metalness{0.0f}
  {
  }

  // Construtor parametrizado para definição completa das propriedades.
  PBRMaterial(const Color& diffuse,
              const Color& specular,
              float r,
              float m):
    Od{diffuse},
    Os{specular},
    roughness{r},
    metalness{m}
  {
  }

  auto clone() const
  {
    return new PBRMaterial{*this};
  }

  // Métodos estáticos de fábrica para metais comuns baseados em valores físicos.
  
  static PBRMaterial* copper(float r = 0.3f)
  {
    return new PBRMaterial(Color{0, 0, 0}, Color{0.95f, 0.64f, 0.54f}, r, 1.0f);
  }

  static PBRMaterial* aluminum(float r = 0.3f)
  {
    return new PBRMaterial(Color{0, 0, 0}, Color{0.91f, 0.92f, 0.92f}, r, 1.0f);
  }

  static PBRMaterial* silver(float r = 0.3f)
  {
    return new PBRMaterial(Color{0, 0, 0}, Color{0.95f, 0.93f, 0.88f}, r, 1.0f);
  }

  static PBRMaterial* titanium(float r = 0.3f)
  {
    return new PBRMaterial(Color{0, 0, 0}, Color{0.542f, 0.497f, 0.449f}, r, 1.0f);
  }

  static PBRMaterial* gold(float r = 0.3f)
  {
    return new PBRMaterial(Color{0, 0, 0}, Color{1.0f, 0.71f, 0.29f}, r, 1.0f);
  }

  // Método de fábrica para materiais dielétricos (não-metais).
  static PBRMaterial* dielectric(const Color& diffuse, float r = 0.5f)
  {
    return new PBRMaterial(diffuse, Color{0.04f, 0.04f, 0.04f}, r, 0.0f);
  }

};

}