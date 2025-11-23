//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2022 Paulo Pagliosa.                              |
//|                                                                 |
//| This software is provided 'as-is', without any express or       |
//| implied warranty. In no event will the authors be held liable   |
//| for any damages arising from the use of this software.          |
//|                                                                 |
//| Permission is granted to anyone to use this software for any    |
//| purpose, including commercial applications, and to alter it and |
//| redistribute it freely, subject to the following restrictions:  |
//|                                                                 |
//| 1. The origin of this software must not be misrepresented; you  |
//| must not claim that you wrote the original software. If you use |
//| this software in a product, an acknowledgment in the product    |
//| documentation would be appreciated but is not required.         |
//|                                                                 |
//| 2. Altered source versions must be plainly marked as such, and  |
//| must not be misrepresented as being the original software.      |
//|                                                                 |
//| 3. This notice may not be removed or altered from any source    |
//| distribution.                                                   |
//|                                                                 |
//[]---------------------------------------------------------------[]
//
// OVERVIEW: MainWindow.h
// ========
// Class definition for PBR application window.
//
// Author: Paulo Pagliosa
// Last revision: 20/11/2025

#ifndef __MainWindow_h
#define __MainWindow_h

#include "graphics/GLRenderWindow3.h"
#include "graphics/GLProgram.h"
#include "graphics/GLMesh.h"
#include "geometry/TriangleMesh.h"
#include "geometry/MeshSweeper.h"
#include "math/Vector3.h"
#include <vector>

using namespace cg;

/////////////////////////////////////////////////////////////////////
//
// Material structure for PBR
//
struct PBRMaterial
{
  vec3f Od;  // Diffuse color
  vec3f Os;  // Specular color (F0)
  float r;   // Roughness (0-1)
  float m;   // Metallic (0-1)
  
  PBRMaterial(const vec3f& od, const vec3f& os, float roughness, float metallic)
    : Od{od}, Os{os}, r{roughness}, m{metallic}
  {
  }
};

/////////////////////////////////////////////////////////////////////
//
// Point Light structure
//
struct PointLight
{
  vec3f position;
  vec3f color;
  float falloff;
  
  PointLight(const vec3f& pos, const vec3f& col, float fo = 1.0f)
    : position{pos}, color{col}, falloff{fo}
  {
  }
};

/////////////////////////////////////////////////////////////////////
//
// Actor structure (sphere)
//
struct Actor
{
  vec3f position;
  PBRMaterial material;
  const TriangleMesh* mesh;
  
  Actor(const vec3f& pos, const PBRMaterial& mat, const TriangleMesh* m)
    : position{pos}, material{mat}, mesh{m}
  {
  }
};


/////////////////////////////////////////////////////////////////////
//
// MainWindow: PBR main window class
// ==========
class MainWindow final: public GLRenderWindow3
{
public:
  MainWindow(int width, int height);

private:
  using Base = GLRenderWindow3;

  // PBR Shader Program
  GLSL::Program _pbrProgram{"PBR Program"};
  GLint _mvMatrixLoc;
  GLint _normalMatrixLoc;
  GLint _mvpMatrixLoc;
  GLint _materialOdLoc;
  GLint _materialOsLoc;
  GLint _materialRoughnessLoc;
  GLint _materialMetallicLoc;
  GLint _lightPositionsLoc;
  GLint _lightColorsLoc;
  GLint _lightFalloffsLoc[3];
  GLint _lightCountLoc;
  
  // Scene data
  Reference<TriangleMesh> _sphereMesh;
  std::vector<Actor> _actors;
  std::vector<PointLight> _lights;
  int _selectedActor{0};
  
  // Camera controls
  vec3f _cameraPos;
  float _cameraFOV{45.0f};
  float _cameraNear{0.1f};
  float _cameraFar{100.0f};
  
  // Overridden methods
  void initialize() override;
  void renderScene() override;
  void gui() override;
  
  // Helper methods
  void initializeShaders();
  void initializeScene();
  void renderActor(const Actor& actor);
  void updateCameraProjection();
  
  static constexpr int NL = 3; // Number of lights
  
}; // MainWindow

#endif // __MainWindow_h
