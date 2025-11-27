//[]---------------------------------------------------------------[]
//|                                                                 |
//| Box.h                                                           |
//|                                                                 |
//| Box shape for TP1 (AABB - Axis-Aligned Bounding Box)           |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"
#include "geometry/Bounds3.h"
#include "graphics/GLGraphics3.h"

namespace cg
{ // begin namespace cg

//
// Box: caixa alinhada aos eixos (AABB)
//
class Box: public Shape3
{
public:
  Box(float size = 2.0f):
    _size{size}
  {
    generateMesh();
  }

  Box(float width, float height, float depth):
    _size{2.0f},
    _width{width},
    _height{height},
    _depth{depth}
  {
    generateMesh();
  }

  float size() const { return _size; }

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
    // Interseção raio-AABB usando método de Bounds3
    // Box centrada na origem com lados de tamanho _size
    float hs = _size / 2.0f;
    Bounds3f bounds{
      vec3f{-hs, -hs, -hs},
      vec3f{hs, hs, hs}
    };
    
    float tMin, tMax;
    if (!bounds.intersect(ray, tMin, tMax))
      return false;
    
    // Escolher menor t positivo dentro do range do raio
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
    // Usar a função box() de GLGraphics3 para obter malha de AABB
    // de lados 2 centrada na origem
    auto boxMesh = GLGraphics3::box();
    if (boxMesh)
    {
      // Copiar dados da malha
      const auto& data = boxMesh->data();
      
      int vertexCount = data.vertexCount;
      int triangleCount = data.triangleCount;
      
      vec3f* vertices = new vec3f[vertexCount];
      vec3f* normals = new vec3f[vertexCount];
      vec2f* uvs = new vec2f[vertexCount];
      TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[triangleCount];
      
      // Copiar vértices e normais
      for (int i = 0; i < vertexCount; ++i)
      {
        vertices[i] = data.vertices[i];
        normals[i] = data.vertexNormals[i];
        uvs[i] = data.uv ? data.uv[i] : vec2f{0.0f, 0.0f};
      }
      
      // Copiar triângulos
      for (int i = 0; i < triangleCount; ++i)
      {
        triangles[i] = data.triangles[i];
      }
      
      TriangleMesh::Data meshData = {
        vertexCount,
        triangleCount,
        vertices,
        normals,
        uvs,
        triangles
      };
      
      _mesh = new TriangleMesh{std::move(meshData)};
    }
  }

}; // Box

} // end namespace cg

