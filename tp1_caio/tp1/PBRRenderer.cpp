//[]---------------------------------------------------------------[]
//|                                                                 |
//| PBRRenderer.cpp                                                 |
//|                                                                 |
//| Physically-Based Rendering renderer for TP1                     |
//|                                                                 |
//[]---------------------------------------------------------------[]

#include "PBRRenderer.h"
#include "PBRActor.h"
// IMPORTANTE: Necessário para GLSL::Program e para a função glMesh()
#include "graphics/GLGraphics3.h" 

// Macro para converter código GLSL em string C++
#define STRINGIFY(A) "#version 400\n"#A

using namespace cg;

/////////////////////////////////////////////////////////////////////
//
// Shaders PBR
// ===========

// Vertex Shader
static const char* pbrVertexShader = STRINGIFY(
  layout(location = 0) in vec4 position;
  layout(location = 1) in vec3 normal;
  
  uniform mat4 mvMatrix;      // Model-View
  uniform mat3 normalMatrix;  // Normal
  uniform mat4 mvpMatrix;     // MVP
  
  out vec3 vPosition;
  out vec3 vNormal;

  void main()
  {
    gl_Position = mvpMatrix * position;
    vPosition = vec3(mvMatrix * position);
    vNormal = normalMatrix * normal;
  }
);

// Fragment Shader
static const char* pbrFragmentShader = STRINGIFY(
  const float PI = 3.14159265359;
  const float MIN_SPEC = 0.04; 

  struct PointLight {
    vec3 position;
    vec3 color;
    int falloff;
  };
  
  struct PBRMaterial {
    vec3 Od;
    vec3 Os;
    float roughness;
    float metalness;
  };
  
  in vec3 vPosition;
  in vec3 vNormal;
  
  uniform PBRMaterial material;
  uniform PointLight lights[3];
  uniform int lightCount;
  
  layout(location = 0) out vec4 fragmentColor;

  // Funções PBR
  vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
  }

  float geometrySchlickGGX(float NdotV, float r) {
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
  }

  float geometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, r) * geometrySchlickGGX(NdotL, r);
  }

  float distributionGGX(vec3 N, vec3 H, float r) {
    float a = r * r;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
  }

  vec3 calculatePBR(vec3 P, vec3 N) {
    N = normalize(N);
    vec3 V = normalize(-P);
    
    vec3 F0 = mix(vec3(MIN_SPEC), material.Os, material.metalness);
    vec3 albedo = material.Od * (1.0 - material.metalness);
    
    vec3 Lo = vec3(0.0);
    
    for(int i = 0; i < lightCount; i++) {
      vec3 L = lights[i].position - P;
      float d = length(L);
      L = normalize(L);
      vec3 H = normalize(V + L);
      
      vec3 radiance = lights[i].color;
      if(lights[i].falloff == 1) radiance /= d;
      else if(lights[i].falloff == 2) radiance /= (d * d);
      
      float NdotL = max(dot(N, L), 0.0);
      if(NdotL > 0.0) {
        float D = distributionGGX(N, H, material.roughness);
        float G = geometrySmith(N, V, L, material.roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 spec = (D * G * F) / (4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001);
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metalness;
        
        Lo += (kD * albedo / PI + spec) * radiance * NdotL;
      }
    }
    return (vec3(0.03) * albedo) + Lo;
  }
  
  void main() {
    vec3 color = calculatePBR(vPosition, vNormal);
    color = color / (color + vec3(1.0)); // Tone mapping
    color = pow(color, vec3(1.0/2.2));   // Gamma
    fragmentColor = vec4(color, 1.0);
  }
);

/////////////////////////////////////////////////////////////////////
//
// PBRRenderer::PBRData
//
struct PBRRenderer::PBRData
{
  struct LightLoc {
    GLint position, color, falloff;
  };
  
  // CORREÇÃO: Isso agora vai compilar porque GLGraphics3.h foi incluído
  GLSL::Program program;
  
  GLint mvMatrixLoc;
  GLint normalMatrixLoc;
  GLint mvpMatrixLoc;
  GLint lightCountLoc;
  LightLoc lightLocs[3];
  
  GLint materialOdLoc;
  GLint materialOsLoc;
  GLint materialRoughnessLoc;
  GLint materialMetalnessLoc;
  
  PBRData();
  void uniformLocations();
}; 

PBRRenderer::PBRData::PBRData(): program{"PBR Program"} {
  program.setShader(GL_VERTEX_SHADER, pbrVertexShader);
  program.setShader(GL_FRAGMENT_SHADER, pbrFragmentShader);
  program.use();
  uniformLocations();
}

void PBRRenderer::PBRData::uniformLocations() {
  mvMatrixLoc = program.uniformLocation("mvMatrix");
  normalMatrixLoc = program.uniformLocation("normalMatrix");
  mvpMatrixLoc = program.uniformLocation("mvpMatrix");
  lightCountLoc = program.uniformLocation("lightCount");
  
  for(int i=0; i<3; ++i) {
    char buf[32];
    snprintf(buf, 32, "lights[%d].position", i); lightLocs[i].position = program.uniformLocation(buf);
    snprintf(buf, 32, "lights[%d].color", i);    lightLocs[i].color = program.uniformLocation(buf);
    snprintf(buf, 32, "lights[%d].falloff", i);  lightLocs[i].falloff = program.uniformLocation(buf);
  }
  
  materialOdLoc = program.uniformLocation("material.Od");
  materialOsLoc = program.uniformLocation("material.Os");
  materialRoughnessLoc = program.uniformLocation("material.roughness");
  materialMetalnessLoc = program.uniformLocation("material.metalness");
}

/////////////////////////////////////////////////////////////////////
//
// PBRRenderer implementation
//
PBRRenderer::PBRRenderer(Scene& scene, Camera& camera):
  _scene{&scene},
  _camera{&camera},
  _viewport{1280, 720},
  _pbrData{new PBRData{}}
{
}

PBRRenderer::~PBRRenderer()
{
  delete _pbrData;
}

void PBRRenderer::update()
{
  glViewport(0, 0, _viewport.w, _viewport.h);
}

void PBRRenderer::beginRender()
{
  const auto& bc = _scene->backgroundColor;
  glClearColor(bc.r, bc.g, bc.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _pbrData->program.use();
}

void PBRRenderer::endRender()
{
  glFlush();
  _pbrData->program.disuse();
}

void PBRRenderer::renderLights()
{
  const auto& vm = _camera->worldToCameraMatrix();
  int nl = 0;
  
  for(auto light : _scene->lights())
  {
    if(!light->isTurnedOn() || nl >= 3) break;
      
    const auto p = vm.transform3x4(light->position());
    
    _pbrData->program.setUniformVec3(_pbrData->lightLocs[nl].position, p);
    _pbrData->program.setUniformVec3(_pbrData->lightLocs[nl].color, vec3f{light->color.r, light->color.g, light->color.b});
    _pbrData->program.setUniform(_pbrData->lightLocs[nl].falloff, (int)light->falloff);
    nl++;
  }
  _pbrData->program.setUniform(_pbrData->lightCountLoc, nl);
}

void PBRRenderer::renderActors()
{
  for(auto actor : _scene->actors())
  {
    if(!actor->isVisible()) continue;

    auto shape = actor->shape();
    if(shape == nullptr) continue;
    
    const auto& mesh = shape->mesh(); 
    if(mesh)
    {
      drawMeshPBR(*mesh, 
                  actor->pbrMaterial(), 
                  actor->transform(),   
                  actor->normalMatrix() 
      );
    }
  }
}

void PBRRenderer::renderMaterial(const PBRMaterial& material)
{
  _pbrData->program.setUniformVec3(_pbrData->materialOdLoc, vec3f{material.Od.r, material.Od.g, material.Od.b});
  _pbrData->program.setUniformVec3(_pbrData->materialOsLoc, vec3f{material.Os.r, material.Os.g, material.Os.b});
  _pbrData->program.setUniform(_pbrData->materialRoughnessLoc, material.roughness);
  _pbrData->program.setUniform(_pbrData->materialMetalnessLoc, material.metalness);
}

void PBRRenderer::drawMeshPBR(const TriangleMesh& mesh,
                         const PBRMaterial& material,
                         const mat4f& t,
                         const mat3f& n)
{
  auto mvm = _camera->worldToCameraMatrix() * t;
  auto mvpm = _camera->projectionMatrix() * mvm;
  auto nm = mat3f{_camera->worldToCameraMatrix()} * n;
  
  _pbrData->program.setUniformMat4(_pbrData->mvMatrixLoc, mvm);
  _pbrData->program.setUniformMat4(_pbrData->mvpMatrixLoc, mvpm);
  _pbrData->program.setUniformMat3(_pbrData->normalMatrixLoc, nm);
  
  renderMaterial(material);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  // Obtém o wrapper OpenGL da malha
  auto m = glMesh(&mesh);
  
  if (m)
  {
      m->bind(); // Faz o binding do VAO
      int indexCount = mesh.data().triangleCount * 3;
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
      // Não precisamos de unbind explícito aqui se o program cuidar disso,
      // mas é boa prática se misturar com outros renderers.
  }
}

void PBRRenderer::render()
{
  update();
  beginRender();
  renderLights();
  renderActors();
  endRender();
}