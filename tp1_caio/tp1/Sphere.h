#pragma once

#include "Shape3.h"
#include <cmath>
#include <map>
#include <vector>
#include <array>
#include <algorithm>

namespace cg
{

// Implementação da primitiva geométrica Esfera.
class Sphere: public Shape3
{
public:
  Sphere(float radius = 1.0f, int subdivisions = 3):
    _radius{radius},
    _subdivisions{subdivisions}
  {
    generateMesh();
  }

  float radius() const { return _radius; }

  // Calcula a normal da superfície no ponto P.
  vec3f normalAt(const vec3f& P) const override
  {
    return P.versor();
  }

  // Verifica interseção entre um raio e a esfera.
  bool intersect(const Ray3f& ray, float& distance) const override
  {
    const vec3f& O = ray.origin;
    const vec3f& D = ray.direction;
    
    // Coeficientes da equação quadrática para interseção raio-esfera.
    float a = D.squaredNorm();
    float b = 2.0f * (O.dot(D));
    float c = O.squaredNorm() - _radius * _radius;
    
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0)
      return false;
    
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    // Considera o menor t positivo.
    float t = t1 > 0 ? t1 : t2;
    
    if (t > 0 && t < distance)
    {
      distance = t;
      return true;
    }
    
    return false;
  }

  // Retorna a Axis-Aligned Bounding Box (AABB) da esfera.
  Bounds3f bounds() const override
  {
    return Bounds3f{
      vec3f{-_radius, -_radius, -_radius},
      vec3f{_radius, _radius, _radius}
    };
  }

protected:
  float _radius;
  int _subdivisions;

  // Gera a malha poligonal via subdivisão recursiva de um icosaedro.
  void generateMesh() override
  {
    std::vector<vec3f> vertices;
    std::vector<std::array<int, 3>> triangles;

    // Definição do icosaedro base.
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    vertices = {
      vec3f{-1,  t,  0}.versor(), vec3f{ 1,  t,  0}.versor(), vec3f{-1, -t,  0}.versor(), vec3f{ 1, -t,  0}.versor(),
      vec3f{ 0, -1,  t}.versor(), vec3f{ 0,  1,  t}.versor(), vec3f{ 0, -1, -t}.versor(), vec3f{ 0,  1, -t}.versor(),
      vec3f{ t,  0, -1}.versor(), vec3f{ t,  0,  1}.versor(), vec3f{-t,  0, -1}.versor(), vec3f{-t,  0,  1}.versor()
    };

    triangles = {
      {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
      {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
      {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    // Processo de subdivisão dos triângulos.
    for (int i = 0; i < _subdivisions; ++i)
    {
      std::vector<std::array<int, 3>> newTriangles;
      std::map<std::pair<int, int>, int> midpointCache;

      auto getMidpoint = [&](int v1, int v2) -> int {
        int a = std::min(v1, v2);
        int b = std::max(v1, v2);
        auto key = std::make_pair(a, b);
        
        auto it = midpointCache.find(key);
        if (it != midpointCache.end()) return it->second;

        vec3f midpoint = ((vertices[v1] + vertices[v2]) * 0.5f).versor();
        int index = (int)vertices.size();
        vertices.push_back(midpoint);
        midpointCache[key] = index;
        return index;
      };

      for (const auto& tri : triangles)
      {
        int v0 = tri[0];
        int v1 = tri[1];
        int v2 = tri[2];
        int a = getMidpoint(v0, v1);
        int b = getMidpoint(v1, v2);
        int c = getMidpoint(v2, v0);

        newTriangles.push_back({v0, a, c});
        newTriangles.push_back({v1, b, a});
        newTriangles.push_back({v2, c, b});
        newTriangles.push_back({a, b, c});
      }
      triangles = newTriangles;
    }

    // Construção e alocação da estrutura TriangleMesh.
    int numVertices = (int)vertices.size();
    int numTriangles = (int)triangles.size();

    vec3f* vertexArray = new vec3f[numVertices];
    vec3f* normalArray = new vec3f[numVertices];
    vec2f* uvArray = new vec2f[numVertices];
    TriangleMesh::Triangle* triangleArray = new TriangleMesh::Triangle[numTriangles];

    for (int i = 0; i < numVertices; ++i)
    {
        vertexArray[i] = vertices[i] * _radius;
        normalArray[i] = vertices[i];
        uvArray[i] = vec2f{0.0f, 0.0f};
    }

    for (int i = 0; i < numTriangles; ++i)
    {
        triangleArray[i].v[0] = triangles[i][0];
        triangleArray[i].v[1] = triangles[i][1];
        triangleArray[i].v[2] = triangles[i][2];
    }

    TriangleMesh::Data data = {
      numVertices,
      numTriangles,
      vertexArray,
      normalArray,
      uvArray,
      triangleArray
    };

    _mesh = new TriangleMesh{std::move(data)};
  }

};

}