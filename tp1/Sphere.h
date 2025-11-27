//[]---------------------------------------------------------------[]
//|                                                                 |
//| Sphere.h                                                   |
//|                                                                 |
//| Sphere shape for TP1                                           |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"
#include "graphics/GLGraphics3.h"
#include <cmath>

namespace cg
{ // begin namespace cg

//
// Sphere: esfera usando malha do GLGraphics3
//
class Sphere: public Shape3
{
public:
  Sphere(float radius = 1.0f):
    _radius{radius}
  {
    generateMesh();
  }

  float radius() const { return _radius; }

  vec3f normalAt(const vec3f& P) const override
  {
    // Para esfera centrada na origem, a normal é P normalizado
    return P.versor();
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Interseção raio-esfera (esfera centrada na origem)
    const vec3f& O = ray.origin;
    const vec3f& D = ray.direction;
    
    float a = D.squaredNorm();
    float b = 2.0f * (O.dot(D));
    float c = O.squaredNorm() - _radius * _radius;
    
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0)
      return false;
    
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    float t = t1 > 0 ? t1 : t2;
    
    if (t > 0 && t < distance)
    {
      distance = t;
      return true;
    }
    
    return false;
  }

  Bounds3f bounds() const override
  {
    return Bounds3f{
      vec3f{-_radius, -_radius, -_radius},
      vec3f{_radius, _radius, _radius}
    };
  }

protected:
  float _radius;

  void generateMesh() override
  {
    // Usar malha do GLGraphics3
    _mesh = GLGraphics3::sphere();
    
    // Escalar para o raio desejado se necessário
    if (_radius != 1.0f)
    {
      auto& data = _mesh->data();
      for (int i = 0; i < data.vertexCount; ++i)
      {
        data.vertices[i] *= _radius;
        // Normais permanecem unitárias
      }
    }
  }

}; // Sphere

} // end namespace cg