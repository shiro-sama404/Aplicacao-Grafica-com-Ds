//[]---------------------------------------------------------------[]
//|                                                                 |
//| Plane.h                                                    |
//|                                                                 |
//| Plane shape for TP1                                            |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"

namespace cg
{ // begin namespace cg

//
// Plane: plano finito (quad)
//
class Plane: public Shape3
{
public:
  Plane(float width = 2.0f, float height = 2.0f):
    _width{width},
    _height{height},
    _normal{0, 1, 0} // Normal aponta para +Y (plano XZ)
  {
    generateMesh();
  }

  // Construtor com normal customizada
  Plane(float width, float height, const vec3f& normal):
    _width{width},
    _height{height},
    _normal{normal.versor()}
  {
    generateMesh();
  }

  vec3f normalAt(const vec3f& P) const override
  {
    // Plano tem normal constante
    return _normal;
  }

  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Interseção raio-plano
    // P = O + tD
    // (P - P0) · N = 0, onde P0 é um ponto no plano (origem)
    // (O + tD - P0) · N = 0
    // t = (P0 - O) · N / (D · N)
    
    const vec3f& O = ray.origin;
    const vec3f& D = ray.direction;
    
    float denom = D.dot(_normal);
    
    if (std::abs(denom) < 1e-6f)
      return false; // Raio paralelo ao plano
    
    // Assumindo plano passa pela origem
    float t = -O.dot(_normal) / denom;
    
    if (t > 0 && t < distance)
    {
      // Verificar se está dentro dos limites do quad
      vec3f P = O + D * t;
      
      // Projetar no plano e verificar bounds
      // Simplificado: assumir plano XZ
      if (std::abs(P.x) <= _width / 2.0f && std::abs(P.z) <= _height / 2.0f)
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
    
    // Assumindo plano XZ na origem
    return Bounds3f{
      vec3f{-hw, -0.01f, -hh},
      vec3f{hw, 0.01f, hh}
    };
  }

protected:
  float _width, _height;
  vec3f _normal;

  void generateMesh() override
  {
    // 1. DEFINIÇÃO DE TAMANHOS
    const int vertexCount = 4;
    const int triangleCount = 2;

    // 2. ALOCAÇÃO DIRETA (Sem construtor auxiliar na TriangleMesh)
    vec3f* vertices = new vec3f[vertexCount];
    vec3f* normals = new vec3f[vertexCount];
    vec2f* uvs = new vec2f;
    TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[triangleCount];

    // 3. PREENCHIMENTO DOS DADOS
    float hw = _width / 2.0f;
    float hh = _height / 2.0f;
    
    // Vértices (Anti-horário para normal +Y)
    vertices[0] = vec3f{-hw, 0, -hh};
    vertices[1] = vec3f{ hw, 0, -hh}; // Note: Ordem ajustada para consistência de winding se necessário
    vertices[2] = vec3f{ hw, 0,  hh};
    vertices[3] = vec3f{-hw, 0,  hh};
    
    for (int i = 0; i < vertexCount; ++i)
      normals[i] = _normal;
    
    // Triângulos
    // T1: 0-1-2
    triangles[0].v[0] = 0; triangles[0].v[1] = 1; triangles[0].v[2] = 2;
    // T2: 0-2-3
    triangles[1].v[0] = 0; triangles[1].v[1] = 2; triangles[1].v[2] = 3;

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

}; // Plane

} // end namespace cg