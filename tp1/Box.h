#pragma once

#include "Shape3.h"
#include "geometry/Bounds3.h"
#include <cstring>
#include <algorithm>

namespace cg
{

class Box: public Shape3
{
public:
  Box(float size = 1.0f):
    _dimensions{size, size, size}
  {
    generateMesh();
  }

  Box(float width, float height, float depth):
    _dimensions{width, height, depth}
  {
    generateMesh();
  }

  // Retorna as dimensões completas (width, height, depth)
  vec3f size() const { return _dimensions; }

  vec3f normalAt(const vec3f& P) const override
  {
    vec3f halfSize = _dimensions * 0.5f;
    vec3f absP = vec3f{std::abs(P.x), std::abs(P.y), std::abs(P.z)};
    
    constexpr float epsilon = 0.001f;

    // Verifica colisão com planos X, Y ou Z independentemente
    if (std::abs(absP.x - halfSize.x) < epsilon)
      return vec3f{P.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f};
      
    if (std::abs(absP.y - halfSize.y) < epsilon)
      return vec3f{0.0f, P.y > 0 ? 1.0f : -1.0f, 0.0f};
      
    if (std::abs(absP.z - halfSize.z) < epsilon)
      return vec3f{0.0f, 0.0f, P.z > 0 ? 1.0f : -1.0f};
    
    return P.versor();
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    vec3f halfSize = _dimensions * 0.5f;
    Bounds3f bounds{ -halfSize, halfSize };
    
    float tMin, tMax;
    if (!bounds.intersect(ray, tMin, tMax))
      return false;
    
    float t = tMin > 0 ? tMin : tMax;
    
    if (t > 0 && t < distance)
    {
      distance = t;
      return true;
    }
    
    return false;
  }

  Bounds3f bounds() const override
  {
    vec3f halfSize = _dimensions * 0.5f;
    return Bounds3f{ -halfSize, halfSize };
  }

protected:
  vec3f _dimensions; 

  void generateMesh() override
  {
    constexpr int nv = 24;
    constexpr int nt = 12;

    // --- DADOS ESTÁTICOS  ---
    static const vec3f rawNormals[nv] = {
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},     // Frente
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, // Trás
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // Esquerda
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},     // Direita
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},     // Topo
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}  // Base
    };

    static const vec3f rawVertices[nv] = {
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1},     // Frente
        {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, // Trás
        {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}, // Esquerda
        {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1},     // Direita
        {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1},     // Topo
        {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}   // Base
    };

    static const int rawIndices[nt * 3] = {
        0, 1, 2, 2, 3, 0,       4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,    12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20
    };

    // --- ALOCAÇÃO ---
    vec3f* vertices = new vec3f[nv];
    vec3f* normals = new vec3f[nv];
    vec2f* uvs = new vec2f[1];
    TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[nt];

    std::memcpy(normals, rawNormals, nv * sizeof(vec3f));
    std::memcpy(triangles, rawIndices, nt * sizeof(TriangleMesh::Triangle));

    vec3f halfSize = _dimensions * 0.5f;

    for (int i = 0; i < nv; ++i)
    {
        vertices[i].x = rawVertices[i].x * halfSize.x;
        vertices[i].y = rawVertices[i].y * halfSize.y;
        vertices[i].z = rawVertices[i].z * halfSize.z;
    }

    TriangleMesh::Data meshData = {
      nv, nt, vertices, normals, uvs, triangles
    };

    _mesh = new TriangleMesh{std::move(meshData)};
  }
};

} // namespace cg