//[]---------------------------------------------------------------[]
//|                                                                 |
//| SphereMeshGenerator.h                                           |
//|                                                                 |
//| Sphere mesh generator for TP1                                  |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "geometry/TriangleMesh.h"
#include <numbers>
#include <vector>
#include <array>
#include <map>

namespace cg
{ // begin namespace cg

//
// SphereMeshGenerator: gerador de malhas esféricas
//
class SphereMeshGenerator
{
public:
  // Gerar malha esférica usando subdivisão UV (latitude/longitude)
  // radius: raio da esfera
  // latitudeDivisions: número de divisões na latitude (mínimo 3)
  // longitudeDivisions: número de divisões na longitude (mínimo 3)
  static TriangleMesh* generate(float radius = 1.0f,
                                int latitudeDivisions = 32,
                                int longitudeDivisions = 32)
  {
    if (latitudeDivisions < 3)
      latitudeDivisions = 3;
    if (longitudeDivisions < 3)
      longitudeDivisions = 3;

    const int vertexCount = (latitudeDivisions + 1) * (longitudeDivisions + 1);
    const int triangleCount = 2 * latitudeDivisions * longitudeDivisions;

    TriangleMesh::Data meshData = {
        vertexCount,
        triangleCount,
        new vec3f[vertexCount],
        new vec3f[vertexCount],
        new vec2f[vertexCount],
        new TriangleMesh::Triangle[triangleCount]
    };

    auto mesh = new TriangleMesh{ std::move(meshData) };
    auto& data = mesh->data();

    // Gerar vértices e normais
    int vertexIndex = 0;
    for (int lat = 0; lat <= latitudeDivisions; ++lat)
    {
      float theta = (float)lat * std::numbers::pi_v<float> / latitudeDivisions;
      float sinTheta = std::sin(theta);
      float cosTheta = std::cos(theta);

      for (int lon = 0; lon <= longitudeDivisions; ++lon)
      {
        float phi = (float)lon * 2.0f * std::numbers::pi_v<float> / longitudeDivisions;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        // Normal (também é a direção do vértice para esfera unitária)
        vec3f normal{sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
        
        // Posição do vértice
        vec3f position = normal * radius;

        data.vertices[vertexIndex] = position;
        data.vertexNormals[vertexIndex] = normal;
        vertexIndex++;
      }
    }

    // Gerar triângulos
    int triangleIndex = 0;
    for (int lat = 0; lat < latitudeDivisions; ++lat)
    {
      for (int lon = 0; lon < longitudeDivisions; ++lon)
      {
        int current = lat * (longitudeDivisions + 1) + lon;
        int next = current + longitudeDivisions + 1;

        // Primeiro triângulo
        data.triangles[triangleIndex].v[0] = current;
        data.triangles[triangleIndex].v[1] = next;
        data.triangles[triangleIndex].v[2] = current + 1;
        triangleIndex++;

        // Segundo triângulo
        data.triangles[triangleIndex].v[0] = current + 1;
        data.triangles[triangleIndex].v[1] = next;
        data.triangles[triangleIndex].v[2] = next + 1;
        triangleIndex++;
      }
    }

    return mesh;
  }

  // Gerar esfera usando subdivisão de icosaedro (mais uniforme)
  // radius: raio da esfera
  // subdivisions: número de subdivisões (0-5 recomendado)
  static TriangleMesh* generateIcosphere(float radius = 1.0f, int subdivisions = 2)
  {
    if (subdivisions < 0)
      subdivisions = 0;
    if (subdivisions > 5)
      subdivisions = 5; // Limitar para evitar explosão de memória

    // Começar com icosaedro
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    
    // 12 vértices do icosaedro
    std::vector<vec3f> vertices = {
      vec3f{-1,  t,  0}.versor(),
      vec3f{ 1,  t,  0}.versor(),
      vec3f{-1, -t,  0}.versor(),
      vec3f{ 1, -t,  0}.versor(),
      vec3f{ 0, -1,  t}.versor(),
      vec3f{ 0,  1,  t}.versor(),
      vec3f{ 0, -1, -t}.versor(),
      vec3f{ 0,  1, -t}.versor(),
      vec3f{ t,  0, -1}.versor(),
      vec3f{ t,  0,  1}.versor(),
      vec3f{-t,  0, -1}.versor(),
      vec3f{-t,  0,  1}.versor()
    };

    // 20 faces do icosaedro
    using Face = std::array<int, 3>;

    // 20 faces do icosaedro
    std::vector<Face> triangles = {
        Face{0, 11, 5}, Face{0, 5, 1}, Face{0, 1, 7}, Face{0, 7, 10}, Face{0, 10, 11},
        Face{1, 5, 9},  Face{5, 11, 4}, Face{11, 10, 2}, Face{10, 7, 6}, Face{7, 1, 8},
        Face{3, 9, 4},  Face{3, 4, 2},  Face{3, 2, 6},  Face{3, 6, 8},  Face{3, 8, 9},
        Face{4, 9, 5},  Face{2, 4, 11}, Face{6, 2, 10}, Face{8, 6, 7},  Face{9, 8, 1}
    };

    // Subdividir
    for (int i = 0; i < subdivisions; ++i)
    {
      std::vector<std::array<int, 3>> newTriangles;
      std::map<std::pair<int, int>, int> midpointCache;

      auto getMidpoint = [&](int v1, int v2) -> int {
        auto key = std::make_pair(std::min(v1, v2), std::max(v1, v2));
        auto it = midpointCache.find(key);
        if (it != midpointCache.end())
          return it->second;

        vec3f midpoint = ((vertices[v1] + vertices[v2]) * 0.5f).versor();
        int index = (int)vertices.size();
        vertices.push_back(midpoint);
        midpointCache[key] = index;
        return index;
      };

      for (const auto& tri : triangles)
      {
        int v0 = tri[0], v1 = tri[1], v2 = tri[2];
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

    int vertexCount = (int)vertices.size();
    int triangleCount = (int)triangles.size();

    TriangleMesh::Data meshData = {
        vertexCount,
        triangleCount,
        new vec3f[vertexCount],
        new vec3f[vertexCount],
        new vec2f[vertexCount],
        new TriangleMesh::Triangle[triangleCount]
    };

    // Criar malha
    auto mesh = new TriangleMesh{(int)vertices.size(), (int)triangles.size()};
    auto& data = mesh->data();

    // Copiar vértices e normais
    for (size_t i = 0; i < vertices.size(); ++i)
    {
      data.vertices[i] = vertices[i] * radius;
      data.vertexNormals[i] = vertices[i]; // Normal é a direção do vértice
    }

    // Copiar triângulos
    for (size_t i = 0; i < triangles.size(); ++i)
    {
      data.triangles[i].v[0] = triangles[i][0];
      data.triangles[i].v[1] = triangles[i][1];
      data.triangles[i].v[2] = triangles[i][2];
    }

    return mesh;
  }

}; // SphereMeshGenerator

} // end namespace cg