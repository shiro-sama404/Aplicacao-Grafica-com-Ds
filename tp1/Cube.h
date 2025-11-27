//[]---------------------------------------------------------------[]
//|                                                                 |
//| Cube.h                                                     |
//|                                                                 |
//| Cube shape for TP1                                             |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"

namespace cg
{ // begin namespace cg

//
// Cube: cubo (box)
//
class Cube: public Shape3
{
public:
  Cube(float size = 1.0f):
    _size{size}
  {
    generateMesh();
  }

  Cube(float width, float height, float depth):
    _size{1.0f},
    _width{width},
    _height{height},
    _depth{depth}
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
    
    // Fallback: normalizar P (aproximação)
    return P.versor();
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Interseção raio-AABB (Axis-Aligned Bounding Box)
    // Algoritmo slab method
    
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
        // Raio paralelo ao slab
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
  float _width, _height, _depth;

  void generateMesh() override
  {
    // 1. DEFINIÇÃO DE TAMANHOS
    // 6 faces * 4 vértices = 24 vértices (normais independentes por face)
    // 6 faces * 2 triângulos = 12 triângulos
    const int vertexCount = 24;
    const int triangleCount = 12;

    // 2. ALOCAÇÃO DIRETA
    vec3f* vertices = new vec3f[vertexCount];
    vec3f* normals = new vec3f[vertexCount];
    vec2f* uvs = new vec2f;
    TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[triangleCount];

    float hs = _size / 2.0f;
    int vIdx = 0;
    int tIdx = 0;

    // Função auxiliar para criar uma face
    auto makeFace = [&](const vec3f& n, 
                        const vec3f& v0, const vec3f& v1, 
                        const vec3f& v2, const vec3f& v3) 
    {
        int base = vIdx;
        
        // Vértices e Normais
        vertices[vIdx] = v0; normals[vIdx] = n; vIdx++;
        vertices[vIdx] = v1; normals[vIdx] = n; vIdx++;
        vertices[vIdx] = v2; normals[vIdx] = n; vIdx++;
        vertices[vIdx] = v3; normals[vIdx] = n; vIdx++;

        // Triângulos (preservando winding CCW: 0-1-2, 0-2-3)
        triangles[tIdx].v[0] = base; 
        triangles[tIdx].v[1] = base + 1; 
        triangles[tIdx].v[2] = base + 2; 
        tIdx++;

        triangles[tIdx].v[0] = base; 
        triangles[tIdx].v[1] = base + 2; 
        triangles[tIdx].v[2] = base + 3; 
        tIdx++;
    };

    // Face +Z (Front)
    makeFace(vec3f{0, 0, 1}, 
             vec3f{-hs, -hs, hs}, vec3f{hs, -hs, hs}, vec3f{hs, hs, hs}, vec3f{-hs, hs, hs});

    // Face -Z (Back) - Ordem ajustada para normal apontar para fora
    makeFace(vec3f{0, 0, -1}, 
             vec3f{hs, -hs, -hs}, vec3f{-hs, -hs, -hs}, vec3f{-hs, hs, -hs}, vec3f{hs, hs, -hs});

    // Face +X (Right)
    makeFace(vec3f{1, 0, 0}, 
             vec3f{hs, -hs, hs}, vec3f{hs, -hs, -hs}, vec3f{hs, hs, -hs}, vec3f{hs, hs, hs});

    // Face -X (Left)
    makeFace(vec3f{-1, 0, 0}, 
             vec3f{-hs, -hs, -hs}, vec3f{-hs, -hs, hs}, vec3f{-hs, hs, hs}, vec3f{-hs, hs, -hs});

    // Face +Y (Top)
    makeFace(vec3f{0, 1, 0}, 
             vec3f{-hs, hs, hs}, vec3f{hs, hs, hs}, vec3f{hs, hs, -hs}, vec3f{-hs, hs, -hs});

    // Face -Y (Bottom)
    makeFace(vec3f{0, -1, 0}, 
             vec3f{-hs, -hs, -hs}, vec3f{hs, -hs, -hs}, vec3f{hs, -hs, hs}, vec3f{-hs, -hs, hs});

    // 4. CRIAÇÃO DA MALHA
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

}; // Cube

} // end namespace cg