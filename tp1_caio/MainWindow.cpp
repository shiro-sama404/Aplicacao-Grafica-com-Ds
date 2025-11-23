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
// OVERVIEW: MainWindow.cpp
// ========
// Source file for PBR application window.
//
// Author: Paulo Pagliosa
// Last revision: 20/11/2025

#include "MainWindow.h"
#include "graphics/GLProgram.h"
#include "graphics/GLMesh.h"
#include "geometry/MeshSweeper.h"
#include "graphics/Camera.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

#define STRINGIFY(A) "#version 400\n"#A

// PBR Vertex Shader
static const char* pbrVertexShader = STRINGIFY(
  layout(location = 0) in vec4 position;
  layout(location = 1) in vec3 normal;
  
  uniform mat4 mvMatrix;
  uniform mat3 normalMatrix;
  uniform mat4 mvpMatrix;
  
  out vec3 vPosition;
  out vec3 vNormal;
  
  void main()
  {
    gl_Position = mvpMatrix * position;
    vPosition = vec3(mvMatrix * position);
    vNormal = normalize(normalMatrix * normal);
  }
);

// PBR Fragment Shader
static const char* pbrFragmentShader = STRINGIFY(
  in vec3 vPosition;
  in vec3 vNormal;
  
  uniform vec3 materialOd;  // Diffuse color
  uniform vec3 materialOs;  // Specular color (F0)
  uniform float materialRoughness;
  uniform float materialMetallic;
  
  uniform vec3 lightPositions[3];
  uniform vec3 lightColors[3];
  uniform float lightFalloffs[3];
  uniform int lightCount;
  
  layout(location = 0) out vec4 fragmentColor;
  
  const float PI = 3.14159265359;
  const float smin = 0.04; // Minimum specular for dielectrics
  
  // Fresnel-Schlick approximation
  vec3 fresnelSchlick(vec3 F0, float cosTheta)
  {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
  }
  
  // Geometry term (Smith's method with Schlick-GGX)
  float geometrySchlickGGX(float NdotV, float kappa)
  {
    float denom = NdotV * (1.0 - kappa) + kappa;
    return NdotV / denom;
  }
  
  float geometrySmith(vec3 N, vec3 V, vec3 L, float kappa)
  {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, kappa);
    float ggx2 = geometrySchlickGGX(NdotL, kappa);
    return ggx1 * ggx2;
  }
  
  // Trowbridge-Reitz (GGX) normal distribution
  float distributionGGX(vec3 N, vec3 H, float alpha)
  {
    float a = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a / max(denom, 0.0000001);
  }
  
  void main()
  {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(-vPosition); // View direction (to camera)
    
    // Interpolate F0 based on metallic
    vec3 F0 = mix(vec3(smin), materialOs, materialMetallic);
    
    // Interpolate diffuse based on metallic (metals have no diffuse)
    vec3 kd = mix(vec3(1.0), vec3(0.0), materialMetallic);
    vec3 Od = materialOd;
    
    // Roughness parameters
    float r = materialRoughness;
    float alpha = r * r;
    float kappa = ((r + 1.0) * (r + 1.0)) / 8.0;
    
    vec3 Lo = vec3(0.0);
    
    // Calculate contribution from each light
    for (int i = 0; i < lightCount && i < 3; i++)
    {
      vec3 Ldir = lightPositions[i] - vPosition;
      float dist = length(Ldir);
      vec3 L = normalize(Ldir);
      
      // Half vector
      vec3 H = normalize(L + V);
      
      // Light attenuation (simplified)
      float attenuation = 1.0 / (1.0 + lightFalloffs[i] * dist * dist);
      vec3 radiance = lightColors[i] * attenuation;
      
      // Diffuse BRDF: fd = Od / PI
      vec3 fd = Od / PI;
      
      // Specular BRDF: fs = (F * G * D) / (4 * (N·L) * (N·V))
      float NdotL = max(dot(N, L), 0.0);
      float NdotV = max(dot(N, V), 0.0);
      
      if (NdotL > 0.0 && NdotV > 0.0)
      {
        // Fresnel
        vec3 F = fresnelSchlick(F0, max(dot(L, H), 0.0));
        
        // Geometry
        float G = geometrySmith(N, V, L, kappa);
        
        // Distribution
        float D = distributionGGX(N, H, alpha);
        
        // Specular BRDF
        vec3 fs = (F * G * D) / (4.0 * NdotL * NdotV);
        
        // Combine diffuse and specular
        vec3 brdf = kd * fd + fs;
        
        Lo += brdf * radiance * NdotL;
      }
    }
    
    // Output final color (multiply by PI as per the equation)
    fragmentColor = vec4(Lo * PI, 1.0);
  }
);


/////////////////////////////////////////////////////////////////////
//
// MainWindow implementation
// ==========
MainWindow::MainWindow(int width, int height):
  Base{"PBR Application", width, height}
{
  // Initialize camera position
  _cameraPos = vec3f{0, 0, 15};
  
  // Initialize lights (3 point lights)
  _lights.push_back(PointLight{vec3f{5, 5, 5}, vec3f{1.0f, 1.0f, 1.0f}, 0.09f});
  _lights.push_back(PointLight{vec3f{-5, 5, -5}, vec3f{0.8f, 0.8f, 1.0f}, 0.09f});
  _lights.push_back(PointLight{vec3f{0, 10, 0}, vec3f{1.0f, 1.0f, 0.9f}, 0.09f});
}

void
MainWindow::initialize()
{
  Base::initialize();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  
  // Create sphere mesh
  _sphereMesh = MeshSweeper::makeSphere(32);
  
  initializeShaders();
  initializeScene();
}

void
MainWindow::initializeShaders()
{
  _pbrProgram.setShaders(pbrVertexShader, pbrFragmentShader);
  _pbrProgram.use();
  
  // Get uniform locations
  _mvMatrixLoc = _pbrProgram.uniformLocation("mvMatrix");
  _normalMatrixLoc = _pbrProgram.uniformLocation("normalMatrix");
  _mvpMatrixLoc = _pbrProgram.uniformLocation("mvpMatrix");
  _materialOdLoc = _pbrProgram.uniformLocation("materialOd");
  _materialOsLoc = _pbrProgram.uniformLocation("materialOs");
  _materialRoughnessLoc = _pbrProgram.uniformLocation("materialRoughness");
  _materialMetallicLoc = _pbrProgram.uniformLocation("materialMetallic");
  _lightPositionsLoc = _pbrProgram.uniformLocation("lightPositions");
  _lightColorsLoc = _pbrProgram.uniformLocation("lightColors");
  // Get locations for array elements
  for (int i = 0; i < 3; i++)
  {
    char name[64];
    snprintf(name, sizeof(name), "lightFalloffs[%d]", i);
    _lightFalloffsLoc[i] = _pbrProgram.uniformLocation(name);
  }
  _lightCountLoc = _pbrProgram.uniformLocation("lightCount");
  
  _pbrProgram.disuse();
}

void
MainWindow::initializeScene()
{
  // Create 12 actors (spheres) - 6 dielectrics on top row, 6 metals on bottom row
  float spacing = 3.0f;
  float startX = -7.5f;
  float topY = 3.0f;
  float bottomY = -3.0f;
  
  // Dielectric materials (top row)
  _actors.push_back(Actor{vec3f{startX, topY, 0}, 
    PBRMaterial{vec3f{0.8f, 0.2f, 0.2f}, vec3f{0.04f, 0.04f, 0.04f}, 0.2f, 0.0f}, _sphereMesh});
  _actors.push_back(Actor{vec3f{startX + spacing, topY, 0}, 
    PBRMaterial{vec3f{0.2f, 0.8f, 0.2f}, vec3f{0.04f, 0.04f, 0.04f}, 0.4f, 0.0f}, _sphereMesh});
  _actors.push_back(Actor{vec3f{startX + 2*spacing, topY, 0}, 
    PBRMaterial{vec3f{0.2f, 0.2f, 0.8f}, vec3f{0.04f, 0.04f, 0.04f}, 0.6f, 0.0f}, _sphereMesh});
  _actors.push_back(Actor{vec3f{startX + 3*spacing, topY, 0}, 
    PBRMaterial{vec3f{0.8f, 0.8f, 0.2f}, vec3f{0.04f, 0.04f, 0.04f}, 0.8f, 0.0f}, _sphereMesh});
  _actors.push_back(Actor{vec3f{startX + 4*spacing, topY, 0}, 
    PBRMaterial{vec3f{0.8f, 0.2f, 0.8f}, vec3f{0.04f, 0.04f, 0.04f}, 0.3f, 0.0f}, _sphereMesh});
  _actors.push_back(Actor{vec3f{startX + 5*spacing, topY, 0}, 
    PBRMaterial{vec3f{0.2f, 0.8f, 0.8f}, vec3f{0.04f, 0.04f, 0.04f}, 0.5f, 0.0f}, _sphereMesh});
  
  // Metal materials (bottom row) - using provided examples
  _actors.push_back(Actor{vec3f{startX, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.95f, 0.64f, 0.54f}, 0.1f, 1.0f}, _sphereMesh}); // Copper
  _actors.push_back(Actor{vec3f{startX + spacing, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.91f, 0.92f, 0.92f}, 0.2f, 1.0f}, _sphereMesh}); // Aluminum
  _actors.push_back(Actor{vec3f{startX + 2*spacing, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.95f, 0.93f, 0.88f}, 0.05f, 1.0f}, _sphereMesh}); // Silver
  _actors.push_back(Actor{vec3f{startX + 3*spacing, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.542f, 0.497f, 0.449f}, 0.3f, 1.0f}, _sphereMesh}); // Titanium
  _actors.push_back(Actor{vec3f{startX + 4*spacing, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{1.0f, 0.71f, 0.29f}, 0.15f, 1.0f}, _sphereMesh}); // Gold
  _actors.push_back(Actor{vec3f{startX + 5*spacing, bottomY, 0}, 
    PBRMaterial{vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.8f, 0.8f, 0.8f}, 0.25f, 1.0f}, _sphereMesh}); // Generic metal
  
  updateCameraProjection();
}

void
MainWindow::updateCameraProjection()
{
  auto cam = camera();
  cam->setPosition(_cameraPos);
  cam->setClippingPlanes(_cameraNear, _cameraFar);
  if (cam->projectionType() == Camera::Perspective)
    cam->setViewAngle(_cameraFOV);
}

void
MainWindow::renderActor(const Actor& actor)
{
  auto cam = camera();
  mat4f model = mat4f::identity();
  model[3] = vec4f{actor.position, 1.0f};
  
  mat4f mv = cam->worldToCameraMatrix() * model;
  // Normal matrix: for pure translation, use camera's rotation part
  // For general case: normalMatrix = (mv^-1)^T, but for pure translation 
  // (no rotation/scale), we can use identity or camera's rotation
  mat3f normalMat = mat3f{cam->worldToCameraMatrix()};
  mat4f mvp = cam->projectionMatrix() * mv;
  
  _pbrProgram.use();
  _pbrProgram.setUniformMat4(_mvMatrixLoc, mv);
  _pbrProgram.setUniformMat3(_normalMatrixLoc, normalMat);
  _pbrProgram.setUniformMat4(_mvpMatrixLoc, mvp);
  _pbrProgram.setUniformVec3(_materialOdLoc, actor.material.Od);
  _pbrProgram.setUniformVec3(_materialOsLoc, actor.material.Os);
  _pbrProgram.setUniform(_materialRoughnessLoc, actor.material.r);
  _pbrProgram.setUniform(_materialMetallicLoc, actor.material.m);
  
  // Set up lights in camera space
  vec3f lightPositions[3];
  vec3f lightColors[3];
  float lightFalloffs[3];
  
  for (int i = 0; i < _lights.size() && i < 3; i++)
  {
    lightPositions[i] = cam->worldToCamera(_lights[i].position);
    lightColors[i] = _lights[i].color;
    lightFalloffs[i] = _lights[i].falloff;
  }
  
  // Set array uniforms using OpenGL directly
  glUniform3fv(_lightPositionsLoc, 3, (float*)lightPositions);
  glUniform3fv(_lightColorsLoc, 3, (float*)lightColors);
  // Set falloff array elements individually
  for (int i = 0; i < 3; i++)
  {
    if (_lightFalloffsLoc[i] >= 0)
      _pbrProgram.setUniform(_lightFalloffsLoc[i], lightFalloffs[i]);
  }
  _pbrProgram.setUniform(_lightCountLoc, (int)_lights.size());
  
  auto mesh = glMesh(actor.mesh);
  mesh->bind();
  glDrawElements(GL_TRIANGLES, mesh->vertexCount(), GL_UNSIGNED_INT, 0);
  
  _pbrProgram.disuse();
}

void
MainWindow::renderScene()
{
  // Render all actors
  for (const auto& actor : _actors)
    renderActor(actor);
}

void
MainWindow::gui()
{
  ImGui::SetNextWindowSize({400, 600}, ImGuiCond_FirstUseEver);
  ImGui::Begin("PBR Controls");
  
  // Camera controls
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
  {
    bool cameraChanged = false;
    cameraChanged |= ImGui::SliderFloat3("Position", (float*)&_cameraPos, -20.0f, 20.0f);
    cameraChanged |= ImGui::SliderFloat("FOV", &_cameraFOV, 10.0f, 120.0f);
    cameraChanged |= ImGui::SliderFloat("Near", &_cameraNear, 0.01f, 10.0f);
    cameraChanged |= ImGui::SliderFloat("Far", &_cameraFar, 10.0f, 200.0f);
    if (cameraChanged)
      updateCameraProjection();
  }
  
  // Light controls
  if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
  {
    for (int i = 0; i < _lights.size() && i < 3; i++)
    {
      char label[32];
      snprintf(label, sizeof(label), "Light %d", i + 1);
      if (ImGui::TreeNode(label))
      {
        ImGui::SliderFloat3("Position", (float*)&_lights[i].position, -20.0f, 20.0f);
        ImGui::ColorEdit3("Color", (float*)&_lights[i].color);
        ImGui::SliderFloat("Falloff", &_lights[i].falloff, 0.0f, 1.0f);
        ImGui::TreePop();
      }
    }
  }
  
  // Material controls
  if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
  {
    int count = (int)_actors.size();
    ImGui::SliderInt("Selected Actor", &_selectedActor, 0, count - 1);
    
    if (_selectedActor >= 0 && _selectedActor < count)
    {
      auto& actor = _actors[_selectedActor];
      ImGui::Text("Actor %d at (%.1f, %.1f, %.1f)", 
        _selectedActor, actor.position.x, actor.position.y, actor.position.z);
      ImGui::Separator();
      
      bool materialChanged = false;
      materialChanged |= ImGui::ColorEdit3("Diffuse (Od)", (float*)&actor.material.Od);
      materialChanged |= ImGui::ColorEdit3("Specular (Os)", (float*)&actor.material.Os);
      materialChanged |= ImGui::SliderFloat("Roughness", &actor.material.r, 0.0f, 1.0f);
      materialChanged |= ImGui::SliderFloat("Metallic", &actor.material.m, 0.0f, 1.0f);
      
      // For metals, ensure Od is (0,0,0)
      if (actor.material.m > 0.99f && (actor.material.Od.x > 0.01f || 
                                       actor.material.Od.y > 0.01f || 
                                       actor.material.Od.z > 0.01f))
      {
        actor.material.Od = vec3f{0, 0, 0};
      }
      // For dielectrics, ensure Os has minimum value
      if (actor.material.m < 0.01f)
      {
        float minOs = std::max({actor.material.Os.x, actor.material.Os.y, actor.material.Os.z});
        if (minOs < 0.04f)
        {
          actor.material.Os = vec3f{0.04f, 0.04f, 0.04f};
        }
      }
    }
  }
  
  ImGui::Separator();
  ImGui::Text("Application: %.3f ms/frame (%.1f FPS)", 
    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  
  ImGui::End();
}
