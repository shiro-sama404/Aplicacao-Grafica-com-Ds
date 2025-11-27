#pragma once

#include "geometry/Ray.h"
#include "geometry/TriangleMesh.h"
#include "graphics/TransformableObject.h"
#include "math/Vector3.h"

namespace cg
{

// Classe base abstrata para representação de primitivas geométricas 3D.
class Shape3 : public TransformableObject
{
public:
  virtual ~Shape3()
  {
    delete _mesh;
  }

  // Retorna a representação em malha triangular da forma.
  TriangleMesh* mesh() const
  {
    return _mesh;
  }

  // Calcula o vetor normal à superfície no ponto P.
  virtual vec3f normalAt(const vec3f& P) const = 0;

  // Realiza o teste de interseção raio-objeto.
  // Se houver interseção mais próxima que 'distance', atualiza o valor e retorna true.
  virtual bool intersect(const Ray3f& ray, float& distance) const = 0;

  // Retorna a Axis-Aligned Bounding Box (AABB) do objeto no espaço local.
  virtual Bounds3f bounds() const = 0;

protected:
  TriangleMesh* _mesh;

  Shape3():
    _mesh{nullptr}
  {
  }

  // Método virtual para geração ou tesselação da malha geométrica.
  virtual void generateMesh() = 0;

};

}