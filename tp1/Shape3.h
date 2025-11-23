//[]---------------------------------------------------------------[]
//|                                                                 |
//| Shape3.h                                                        |
//|                                                                 |
//| Abstract 3D shape class for TP1                                |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "geometry/Ray.h"
#include "geometry/TriangleMesh.h"
#include "graphics/TransformableObject.h"
#include "math/Vector3.h"

namespace cg
{ // begin namespace cg

//
// Shape3: classe abstrata para formas geométricas 3D
//
class Shape3 : public TransformableObject
{
public:
  virtual ~Shape3()
  {
    delete _mesh;
  }

  // Obter malha triangular
  TriangleMesh* mesh() const
  {
    return _mesh;
  }

  // Normal na superfície em um ponto P
  // P deve estar na superfície da forma
  virtual vec3f normalAt(const vec3f& P) const = 0;

  // Teste de interseção com raio
  // ray: raio a testar
  // distance: [in/out] distância máxima/distância da interseção
  // Retorna true se houver interseção dentro da distância
  virtual bool intersect(const Ray3f& ray, float& distance) const = 0;

  // Bounds (opcional, para otimizações)
  virtual Bounds3f bounds() const = 0;

protected:
  TriangleMesh* _mesh;

  Shape3():
    _mesh{nullptr}
  {
    // do nothing
  }

  // Gerar malha (chamado pelas subclasses)
  virtual void generateMesh() = 0;

}; // Shape3

} // end namespace cg