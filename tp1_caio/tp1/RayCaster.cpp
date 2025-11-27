//[]---------------------------------------------------------------[]
//|                                                                 |
//| RayCaster.cpp                                                   |
//|                                                                 |
//| Ray casting renderer implementation with PBR lighting          |
//|                                                                 |
//[]---------------------------------------------------------------[]

#include "RayCaster.h"
#include "graphics/Light.h"
#include <cmath>
#include <limits>

namespace cg
{ // begin namespace cg

constexpr float PI = 3.14159265359f;
constexpr float MIN_SPEC = 0.04f;
constexpr float EPSILON = 1e-4f;

void
RayCaster::buildBVH()
{
  // Criar array de atores para BVH
  std::vector<Reference<PBRActor>> actors;
  
  for (auto actor : _scene->actors())
  {
    if (actor->isVisible())
      actors.push_back(actor);
  }
  
  printf("Building BVH with %zu actors\n", actors.size());
  
  if (!actors.empty())
  {
    _bvh = new BVH<PBRActor>{std::move(actors), 8, BVHBase::SplitMethod::SAH};
    printf("BVH built successfully. Bounds: ");
    auto bounds = _bvh->bounds();
    bounds.min().print("min: ");
    bounds.max().print("max: ");
  }
  else
  {
    printf("Warning: No visible actors to build BVH!\n");
  }
}

void
RayCaster::setPixelRay(float x, float y, Ray3f& ray)
{
  auto p = imageToWindow(x, y);
  const auto& m = _camera->cameraToWorldMatrix();
  
  // VRC axes (extrair apenas xyz dos vetores 4D)
  vec3f u = vec3f{m[0].x, m[0].y, m[0].z};
  vec3f v = vec3f{m[1].x, m[1].y, m[1].z};
  vec3f n = vec3f{m[2].x, m[2].y, m[2].z};
  
  float F, B;
  _camera->clippingPlanes(F, B);
  
  if (_camera->projectionType() == Camera::Perspective)
  {
    ray.origin = _camera->position();
    ray.direction = (p - _camera->nearPlane() * n).versor();
  }
  else
  {
    ray.origin = _camera->position() + p;
    ray.direction = -n;
  }
  
  ray.tMin = F;
  ray.tMax = B;
}

vec3f
RayCaster::imageToWindow(float x, float y) const
{
  const auto& m = _camera->cameraToWorldMatrix();
  vec3f u = vec3f{m[0].x, m[0].y, m[0].z};
  vec3f v = vec3f{m[1].x, m[1].y, m[1].z};
  
  float wh = _camera->windowHeight();
  float Iw = 1.0f / _viewport.w;
  float Ih = 1.0f / _viewport.h;
  
  float Vw, Vh;
  if (_viewport.w >= _viewport.h)
    Vw = (Vh = wh) * _viewport.w * Ih;
  else
    Vh = (Vw = wh) * _viewport.h * Iw;
  
  return Vw * (x * Iw - 0.5f) * u + Vh * (y * Ih - 0.5f) * v;
}

bool
RayCaster::intersect(const Ray3f& ray, Intersection& hit)
{
  hit.object = nullptr;
  hit.distance = ray.tMax;
  
  if (_bvh == nullptr || _bvh->empty())
  {
    printf("BVH is null or empty in intersect()\n");
    return false;
  }
  
  // Usar BVH para encontrar interseção
  // O BVH trabalha com PBRActor que já tem métodos intersect implementados
  bool found = _bvh->intersect(ray, hit);
  
  if (!found)
  {
    // Tentar busca linear como fallback para debug
    printf("BVH intersect returned false. Trying linear search...\n");
    PBRActor* closestActor = nullptr;
    float closestDistance = ray.tMax;
    
    for (auto actor : _scene->actors())
    {
      if (!actor->isVisible())
        continue;
      
      Intersection tempHit;
      tempHit.distance = closestDistance;
      if (actor->intersect(ray, tempHit))
      {
        if (tempHit.distance < closestDistance)
        {
          closestDistance = tempHit.distance;
          closestActor = actor;
        }
      }
    }
    
    if (closestActor != nullptr)
    {
      printf("Linear search found actor: %s at distance %f\n", closestActor->name(), closestDistance);
      hit.object = closestActor;
      hit.distance = closestDistance;
      return true;
    }
    else
    {
      printf("Linear search also found nothing\n");
    }
  }
  
  return found;
}

Color
RayCaster::calculatePBR(const vec3f& P, const vec3f& N, const PBRMaterial& material)
{
  vec3f V = (_camera->position() - P).versor();
  vec3f normal = N.versor();
  
  // F0 para Fresnel
  vec3f F0 = vec3f{material.Os.r, material.Os.g, material.Os.b} * material.metalness + vec3f{MIN_SPEC, MIN_SPEC, MIN_SPEC} * (1.0f - material.metalness);
  
  // Albedo
  vec3f albedo = vec3f{material.Od.r, material.Od.g, material.Od.b} * (1.0f - material.metalness);
  
  Color Lo{0, 0, 0};
  
  // Iluminação de todas as luzes
  for (auto light : _scene->lights())
  {
    if (!light->isTurnedOn())
      continue;
    
    vec3f L;
    float d;
    
    if (!light->lightVector(P, L, d))
      continue;
    
    float NdotL = normal.dot(L);
    if (NdotL <= 0)
      continue;
    
    // Verificar sombras
    Ray3f shadowRay{P + L * EPSILON, L};
    shadowRay.tMax = d;
    
    Intersection shadowHit;
    if (intersect(shadowRay, shadowHit))
      continue;
    
    // Calcular radiance
    Color radiance = light->lightColor(d);
    
    // Half vector
    vec3f H = (V + L).versor();
    float NdotV = std::max(normal.dot(V), 0.0f);
    float NdotH = std::max(normal.dot(H), 0.0f);
    float VdotH = std::max(V.dot(H), 0.0f);
    
    // Roughness
    float a = material.roughness * material.roughness;
    float a2 = a * a;
    
    // Distribution GGX (D)
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    float D = a2 / (PI * denom * denom);
    
    // Geometry Smith (G)
    float k = (material.roughness + 1.0f) * (material.roughness + 1.0f) / 8.0f;
    float G1V = NdotV / (NdotV * (1.0f - k) + k);
    float G1L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G1V * G1L;
    
    // Fresnel Schlick (F)
    vec3f F = F0 + (vec3f{1, 1, 1} - F0) * std::pow(1.0f - VdotH, 5.0f);
    
    // Specular BRDF
    float denomSpec = 4.0f * NdotV * NdotL + 1e-6f;
    vec3f specular = F * (D * G / denomSpec);
    
    // kS e kD
    vec3f kS = F;
    vec3f kD = (vec3f{1, 1, 1} - kS) * (1.0f - material.metalness);
    
    // Contribuição da luz
    vec3f radianceVec = vec3f{radiance.r, radiance.g, radiance.b};
    vec3f diffuseTerm = kD * albedo * (1.0f / PI);
    vec3f contribution = (diffuseTerm + specular) * radianceVec * NdotL;
    Lo.r += contribution.x;
    Lo.g += contribution.y;
    Lo.b += contribution.z;
  }
  
  // Ambiente
  vec3f ambient = vec3f{0.03f, 0.03f, 0.03f} * albedo;
  Lo.r += ambient.x;
  Lo.g += ambient.y;
  Lo.b += ambient.z;
  
  return Lo;
}

Color
RayCaster::shade(const Ray3f& ray, const Intersection& hit)
{
  auto actor = (PBRActor*)hit.object;
  if (actor == nullptr)
    return background();
  
  // Calcular ponto de interseção
  vec3f P = ray(hit.distance);
  
  // Obter shape e calcular normal
  auto shape = actor->shape();
  if (shape == nullptr)
    return background();
  
  // Transformar ponto para espaço local
  const auto& invTransform = actor->inverseTransform();
  vec3f localP = invTransform.transform3x4(P);
  
  // Calcular normal no espaço local
  vec3f localN = shape->normalAt(localP);
  
  // Transformar normal para espaço global
  const auto& normalMatrix = actor->normalMatrix();
  // Multiplicar matriz 3x3 por vetor 3D
  vec3f N = vec3f{
    normalMatrix[0].dot(localN),
    normalMatrix[1].dot(localN),
    normalMatrix[2].dot(localN)
  }.versor();
  
  // Obter material
  const auto& material = actor->pbrMaterial();
  
  // Calcular cor PBR
  return calculatePBR(P, N, material);
}

Color
RayCaster::trace(const Ray3f& ray, int depth)
{
  if (depth > 5) // Limite de recursão
    return background();
  
  Intersection hit;
  if (!intersect(ray, hit))
    return background();
  
  return shade(ray, hit);
}

Color
RayCaster::shoot(float x, float y)
{
  Ray3f ray;
  setPixelRay(x, y, ray);
  Color color = trace(ray, 0);
  
  // Clamp e tone mapping simples
  color.r = std::min(color.r, 1.0f);
  color.g = std::min(color.g, 1.0f);
  color.b = std::min(color.b, 1.0f);
  
  return color;
}

Color
RayCaster::background() const
{
  return _scene->backgroundColor;
}

void
RayCaster::renderImage(Image& image)
{
  ImageBuffer scanLine{_viewport.w, 1};
  
  for (int j = 0; j < _viewport.h; j++)
  {
    auto y = (float)j + 0.5f;
    printf("Scanning line %d of %d\r", j + 1, _viewport.h);
    
    for (int i = 0; i < _viewport.w; i++)
    {
      scanLine[i] = shoot((float)i + 0.5f, y);
    }
    
    image.setData(0, j, scanLine);
  }
  
  printf("\n");
}

PBRActor*
RayCaster::selectActor(int x, int y)
{
  if (_bvh == nullptr || _bvh->empty())
  {
    printf("BVH is null or empty! Rebuilding...\n");
    buildBVH();
    if (_bvh == nullptr || _bvh->empty())
    {
      printf("Still no BVH after rebuild!\n");
      return nullptr;
    }
  }
  
  Ray3f ray;
  setPixelRay((float)x + 0.5f, (float)y + 0.5f, ray);
  
  printf("Ray origin: ");
  ray.origin.print("");
  printf("Ray direction: ");
  ray.direction.print("");
  printf("Ray tMin: %f, tMax: %f\n", ray.tMin, ray.tMax);
  
  Intersection hit;
  hit.distance = ray.tMax;
  
  if (intersect(ray, hit))
  {
    printf("Intersection found! Distance: %f\n", hit.distance);
    return (PBRActor*)hit.object;
  }
  
  printf("No intersection found\n");
  return nullptr;
}

} // end namespace cg

