//[]---------------------------------------------------------------[]
//|                                                                 |
//| Plane.h                                                         |
//|                                                                 |
//| Plane shape for TP1 (Standard XY Plane)                         |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"

namespace cg
{ // begin namespace cg

class Plane: public Shape3
{
public:
  Plane(float width = 2.0f, float height = 2.0f):
    _width{width},
    _height{height},
    _normal{0, 0, 1} // Normal padrão +Z
  {
    generateMesh();
  }

  // Métodos de interseção e normal devem considerar o plano local XY
  vec3f normalAt(const vec3f& P) const override
  {
    return _normal;
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Interseção com plano Z=0
    if (std::abs(ray.direction.z) < 1e-6f) return false;
    
    float t = -ray.origin.z / ray.direction.z;
    
    if (t > 0 && t < distance)
    {
      vec3f P = ray(t);
      // Verificar limites em X e Y
      if (std::abs(P.x) <= _width/2.0f && std::abs(P.y) <= _height/2.0f)
      {
        distance = t;
        return true;
      }
    }
    return false;
  }

  Bounds3f bounds() const override
  {
    float hw = _width / 2.0f;
    float hh = _height / 2.0f;
    // Bounds finos no eixo Z
    return Bounds3f{
      vec3f{-hw, -hh, -0.01f},
      vec3f{hw, hh, 0.01f}
    };
  }

protected:
  float _width, _height;
  vec3f _normal;

  void generateMesh() override
  {
    const int vertexCount = 4;
    const int triangleCount = 2;

    vec3f* vertices = new vec3f[vertexCount];
    vec3f* normals = new vec3f[vertexCount];
    vec2f* uvs = new vec2f[vertexCount];
    TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[triangleCount];

    float hw = _width / 2.0f;
    float hh = _height / 2.0f;
    
    // Vértices no plano XY (em pé)
    // Ordem CCW (Counter-Clockwise)
    vertices[0] = vec3f{-hw, -hh, 0}; // Bottom-Left
    vertices[1] = vec3f{ hw, -hh, 0}; // Bottom-Right
    vertices[2] = vec3f{ hw,  hh, 0}; // Top-Right
    vertices[3] = vec3f{-hw,  hh, 0}; // Top-Left
    
    for (int i = 0; i < vertexCount; ++i)
    {
        normals[i] = _normal; // +Z
        uvs[i] = vec2f{0, 0};
    }
    
    // Triângulos CCW
    triangles[0].v[0] = 0; triangles[0].v[1] = 1; triangles[0].v[2] = 2;
    triangles[1].v[0] = 0; triangles[1].v[1] = 2; triangles[1].v[2] = 3;

    TriangleMesh::Data data = {
      vertexCount,
      triangleCount,
      vertices,
      normals,
      uvs,
      triangles
    };

    _mesh = new TriangleMesh{std::move(data)};
  }

}; // Plane

} // end namespace cg