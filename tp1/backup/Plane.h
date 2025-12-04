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
    // Verifica paralelismo com o plano y=0.
    if (std::abs(ray.direction.y) < 1e-6f) return false;
    
    float t = -ray.origin.y / ray.direction.y;
    
    if (t > 0 && t < distance)
    {
      vec3f P = ray.origin + ray.direction * t;;
      // Validação das coordenadas locais (hit point bounds).
      if (std::abs(P.x) <= _width/2.0f && std::abs(P.y) <= _height/2.0f)
      {
        distance = t;
        return true;
      }
    }
    return false;
  }

  // Define a AABB com espessura mínima no eixo y.
  Bounds3f bounds() const override
  {
    float hw = _width / 2.0f;
    float hh = _height / 2.0f;
    return Bounds3f{
      vec3f{-hw, -hh, -0.01f},
      vec3f{hw, hh, 0.01f}
    };
  }

private:
  float _width, _height;
  vec3f _normal = {0.0f, 1.0f, 0.0f};

  void generateMesh() override
  {
    const int vertexCount = 4;
    const int triangleCount = 2;

    vec3f* vertices = new vec3f[vertexCount];
    vec3f* normals = new vec3f[vertexCount];
    vec2f* uvs = new vec2f[1];
    TriangleMesh::Triangle* triangles = new TriangleMesh::Triangle[triangleCount];

    float hw = _width / 2.0f;
    float hh = _height / 2.0f;

    vertices[0] = vec3f{-hw, 0.0f, -hh}; // Inferior Esquerdo
    vertices[1] = vec3f{ hw, 0.0f, -hh}; // Inferior Direito
    vertices[2] = vec3f{ hw, 0.0f,  hh}; // Superior Direito
    vertices[3] = vec3f{-hw, 0.0f,  hh}; // Superior Esquerdo
    
    for (int i = 0; i < vertexCount; ++i)
      normals[i] = _normal;
    
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