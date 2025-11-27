//[]---------------------------------------------------------------[]
//|                                                                 |
//| Cube.h                                                     |
//|                                                                 |
//| Cube shape for TP1                                             |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"
#include "graphics/GLGraphics3.h"

namespace cg
{ // begin namespace cg

//
// Cube: cubo usando malha do GLGraphics3
//
class Cube: public Shape3
{
public:
  Cube(float size = 1.0f):
    _size{size}
  {
    generateMesh();
  }

  vec3f normalAt(const vec3f& P) const override
  {
    // Determinar qual face baseado em qual componente tem maior magnitude
    vec3f absP = vec3f{std::abs(P.x), std::abs(P.y), std::abs(P.z)};
    float hs = _size / 2.0f;
    
    if (std::abs(absP.x - hs) < 0.001f)
      return vec3f{P.x > 0 ? 1.0f : -1.0f, 0, 0};
    if (std::abs(absP.y - hs) < 0.001f)
      return vec3f{0, P.y > 0 ? 1.0f : -1.0f, 0};
    if (std::abs(absP.z - hs) < 0.001f)
      return vec3f{0, 0, P.z > 0 ? 1.0f : -1.0f};
    
    // Fallback: normalizar P
    return P.versor();
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Interseção raio-AABB (slab method)
    float hs = _size / 2.0f;
    vec3f minBounds{-hs, -hs, -hs};
    vec3f maxBounds{hs, hs, hs};
    
    const vec3f& O = ray.origin;
    const vec3f& D = ray.direction;
    
    float tmin = -std::numeric_limits<float>::infinity();
    float tmax = std::numeric_limits<float>::infinity();
    
    for (int i = 0; i < 3; ++i)
    {
      if (std::abs(D[i]) < 1e-6f)
      {
        if (O[i] < minBounds[i] || O[i] > maxBounds[i])
          return false;
      }
      else
      {
        float t1 = (minBounds[i] - O[i]) / D[i];
        float t2 = (maxBounds[i] - O[i]) / D[i];
        
        if (t1 > t2)
          std::swap(t1, t2);
        
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        
        if (tmin > tmax)
          return false;
      }
    }
    
    float t = tmin > 0 ? tmin : tmax;
    
    if (t > 0 && t < distance)
    {
      distance = t;
      return true;
    }
    
    return false;
  }

  Bounds3f bounds() const override
  {
    float hs = _size / 2.0f;
    return Bounds3f{
      vec3f{-hs, -hs, -hs},
      vec3f{hs, hs, hs}
    };
  }

protected:
  float _size;

  void generateMesh() override
  {
    // Usar malha do GLGraphics3
    _mesh = GLGraphics3::box();
    
    // Escalar para o tamanho desejado se necessário
    if (_size != 1.0f)
    {
      auto& data = _mesh->data();
      for (int i = 0; i < data.vertexCount; ++i)
      {
        data.vertices[i] *= _size;
        // Normais permanecem unitárias
      }
    }
  }

}; // Cube

} // end namespace cg