#pragma once

#include "Shape3.h"

namespace cg
{

class Plane: public Shape3
{
public:
  Plane(float width = 2.0f, float height = 2.0f):
    _width{width},
    _height{height},
    _normal{0, 0, 1} // Normal padrão orientada para +Z.
  {
    generateMesh();
  }

  // Retorna a normal constante da superfície plana.
  vec3f normalAt(const vec3f& P) const override
  {
    return _normal;
  }

  // Realiza o teste de interseção raio-plano.
  // Verifica se o ponto de interseção no plano infinito reside dentro dos limites de largura e altura.
  bool intersect(const Ray3f& ray, float& distance) const override
  {
    // Verifica paralelismo com o plano Z=0.
    if (std::abs(ray.direction.z) < 1e-6f) return false;
    
    float t = -ray.origin.z / ray.direction.z;
    
    if (t > 0 && t < distance)
    {
      vec3f P = ray(t);
      // Validação das coordenadas locais (hit point bounds).
      if (std::abs(P.x) <= _width/2.0f && std::abs(P.y) <= _height/2.0f)
      {
        distance = t;
        return true;
      }
    }
    return false;
  }

  // Define a AABB com espessura mínima no eixo Z para garantir consistência em estruturas espaciais.
  Bounds3f bounds() const override
  {
    float hw = _width / 2.0f;
    float hh = _height / 2.0f;
    return Bounds3f{
      vec3f{-hw, -hh, -0.01f},
      vec3f{hw, hh, 0.01f}
    };
  }

protected:
  float _width, _height;
  vec3f _normal;

  // Gera a malha geométrica composta por dois triângulos (quad).
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
    
    // Definição dos vértices em ordem anti-horária (CCW).
    vertices[0] = vec3f{-hw, -hh, 0}; // Inferior Esquerdo
    vertices[1] = vec3f{ hw, -hh, 0}; // Inferior Direito
    vertices[2] = vec3f{ hw,  hh, 0}; // Superior Direito
    vertices[3] = vec3f{-hw,  hh, 0}; // Superior Esquerdo
    
    for (int i = 0; i < vertexCount; ++i)
    {
        normals[i] = _normal;
        uvs[i] = vec2f{0, 0};
    }
    
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

};

}