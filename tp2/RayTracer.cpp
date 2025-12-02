//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2018, 2023 Paulo Pagliosa.                        |
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
// OVERVIEW: RayTracer.cpp
// ========
// Source file for simple ray tracer.
//
// Author: Paulo Pagliosa
// Last revision: 30/07/2023

#include "graphics/Camera.h"
#include "utils/Stopwatch.h"
#include "RayTracer.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;

namespace cg
{ // begin namespace cg

namespace
{ // begin namespace

inline void
printElapsedTime(const char* s, Stopwatch::ms_time time)
{
  printf("%sElapsed time: %g ms\n", s, time);
}

inline auto
arand()
{
  return ((float)std::rand() / RAND_MAX / 4.0f) - 0.125f;
}

inline auto
maxabs(const Color& c)
{
  return math::max(math::max(std::abs(c.r), std::abs(c.g)), std::abs(c.b));
}

} // end namespace


/////////////////////////////////////////////////////////////////////
//
// RayTracer implementation
// =========
RayTracer::RayTracer(SceneBase& scene, Camera& camera):
  Renderer{scene, camera},
  _maxRecursionLevel{6},
  _minWeight{minMinWeight}
{
  _supersamplingParams.adaptiveDistance = 0.1f;
  _supersamplingParams.maxSubdivisionLevel = 2;
  _supersamplingParams.enabled = false;
}

void
RayTracer::update()
{
  // Delete current BVH before creating a new one
  _bvh = nullptr;

  PrimitiveBVH::PrimitiveArray primitives;
  auto np = uint32_t(0);

  primitives.reserve(_scene->actorCount());
  for (auto actor : _scene->actors())
    if (actor->visible)
    {
      auto p = actor->mapper()->primitive();

      assert(p != nullptr);
      if (p->canIntersect())
      {
        primitives.push_back(p);
        np++;
      }
    }
  _bvh = new PrimitiveBVH{std::move(primitives)};
}

void
RayTracer::render()
{
  throw std::runtime_error("RayTracer::render() invoked");
}

void
RayTracer::renderImage(Image& image)
{
  Stopwatch timer;

  update();
  timer.start();
  {
    const auto& m = _camera->cameraToWorldMatrix();

    // VRC axes
    _vrc.u = m[0];
    _vrc.v = m[1];
    _vrc.n = m[2];
  }

  // init auxiliary mapping variables
  auto w = image.width(), h = image.height();

  setImageSize(w, h);
  _Iw = math::inverse(float(w));
  _Ih = math::inverse(float(h));
  {
    auto wh = _camera->windowHeight();

    if (w >= h)
      _Vw = (_Vh = wh) * w * _Ih;
    else
      _Vh = (_Vw = wh) * h * _Iw;
  }

  // init pixel ray
  float F, B;

  _camera->clippingPlanes(F, B);
  if (_camera->projectionType() == Camera::Perspective)
  {
    // distance from the camera position to a frustum back corner
    auto z = B / F * 0.5f;
    B = vec3f{_Vw * z, _Vh * z, B}.length();
  }
  _pixelRay.tMin = F;
  _pixelRay.tMax = B;
  _pixelRay.set(_camera->position(), -_vrc.n);
  _numberOfRays = _numberOfHits = 0;
  _rayColorCache.clear();
  scan(image);

  auto et = timer.time();

  std::cout << "\nNumber of rays: " << _numberOfRays;
  std::cout << "\nNumber of hits: " << _numberOfHits;
  printElapsedTime("\nDONE! ", et);
}

void
RayTracer::setPixelRay(float x, float y)
//[]---------------------------------------------------[]
//|  Set pixel ray                                      |
//|  @param x coordinate of the pixel                   |
//|  @param y cordinates of the pixel                   |
//[]---------------------------------------------------[]
{
  auto p = imageToWindow(x, y);

  switch (_camera->projectionType())
  {
    case Camera::Perspective:
      _pixelRay.direction = (p - _camera->nearPlane() * _vrc.n).versor();
      break;

    case Camera::Parallel:
      _pixelRay.origin = _camera->position() + p;
      break;
  }
}

void
RayTracer::scan(Image& image)
{
  if (_supersamplingParams.enabled && _supersamplingParams.maxSubdivisionLevel > 0)
  {
    // Adaptive supersampling
    ImageBuffer scanLine{_viewport.w, 1};
    
    for (auto j = 0; j < _viewport.h; j++)
    {
      printf("Scanning line %d of %d\r", j + 1, _viewport.h);
      
      for (auto i = 0; i < _viewport.w; i++)
      {
        float x0 = (float)i;
        float y0 = (float)j;
        float x1 = x0 + 1.0f;
        float y1 = y0 + 1.0f;
        
        scanLine[i] = adaptiveSupersample(x0, y0, x1, y1, 0);
      }
      
      image.setData(0, j, scanLine);
    }
  }
  else
  {
    // Standard single ray per pixel
    ImageBuffer scanLine{_viewport.w, 1};

    for (auto j = 0; j < _viewport.h; j++)
    {
      auto y = (float)j + 0.5f;

      printf("Scanning line %d of %d\r", j + 1, _viewport.h);
      for (auto i = 0; i < _viewport.w; i++)
        scanLine[i] = shoot((float)i + 0.5f, y);
      image.setData(0, j, scanLine);
    }
  }
}

Color
RayTracer::shoot(float x, float y)
//[]---------------------------------------------------[]
//|  Shoot a pixel ray                                  |
//|  @param x coordinate of the pixel                   |
//|  @param y cordinates of the pixel                   |
//|  @return RGB color of the pixel                     |
//[]---------------------------------------------------[]
{
  // set pixel ray
  setPixelRay(x, y);

  // trace pixel ray (starting in air/vacuum with IOR = 1.0)
  Color color = trace(_pixelRay, 0, 1, 1.0f);

  // adjust RGB color
  if (color.r > 1.0f)
    color.r = 1.0f;
  if (color.g > 1.0f)
    color.g = 1.0f;
  if (color.b > 1.0f)
    color.b = 1.0f;
  // return pixel color
  return color;
}

Color
RayTracer::getRayColor(float x, float y, bool useJitter, float subpixelSize)
//[]---------------------------------------------------[]
//|  Get color for a ray at position (x, y)            |
//|  Uses cache to avoid recomputing shared rays        |
//[]---------------------------------------------------[]
{
  float finalX = x;
  float finalY = y;
  
  if (useJitter)
  {
    float jitterX = arand() * subpixelSize;
    float jitterY = arand() * subpixelSize;
    finalX += jitterX;
    finalY += jitterY;
  }
  
  // Check cache first
  uint64_t key = hashRayPosition(finalX, finalY);
  auto it = _rayColorCache.find(key);
  if (it != _rayColorCache.end())
    return it->second;
  
  // Compute ray color
  setPixelRay(finalX, finalY);
  Color color = trace(_pixelRay, 0, 1, 1.0f);
  
  // Clamp color
  if (color.r > 1.0f) color.r = 1.0f;
  if (color.g > 1.0f) color.g = 1.0f;
  if (color.b > 1.0f) color.b = 1.0f;
  
  // Cache the result
  _rayColorCache[key] = color;
  
  return color;
}

Color
RayTracer::adaptiveSupersample(float x0, float y0, float x1, float y1, uint32_t level)
//[]---------------------------------------------------[]
//|  Adaptive supersampling for a pixel region          |
//|  @param x0, y0 - top-left corner                    |
//|  @param x1, y1 - bottom-right corner                |
//|  @param level - current subdivision level           |
//[]---------------------------------------------------[]
{
  if (level >= _supersamplingParams.maxSubdivisionLevel)
  {
    // Max level reached, sample center
    float cx = (x0 + x1) * 0.5f;
    float cy = (y0 + y1) * 0.5f;
    float subpixelSize = std::max(x1 - x0, y1 - y0);
    return getRayColor(cx, cy, true, subpixelSize);
  }
  
  // Sample four corners (with jitter only at first level, corners should be exact for sharing)
  float subpixelSize = std::max(x1 - x0, y1 - y0);
  bool useJitter = (level == 0); // Only jitter at pixel level, not subpixel level for corner sharing
  Color c00 = getRayColor(x0, y0, useJitter, subpixelSize);
  Color c01 = getRayColor(x0, y1, useJitter, subpixelSize);
  Color c10 = getRayColor(x1, y0, useJitter, subpixelSize);
  Color c11 = getRayColor(x1, y1, useJitter, subpixelSize);
  
  // Compute average
  Color average = (c00 + c01 + c10 + c11) * 0.25f;
  
  // Compute deviations from average
  float d00 = maxabs(c00 - average);
  float d01 = maxabs(c01 - average);
  float d10 = maxabs(c10 - average);
  float d11 = maxabs(c11 - average);
  
  // Check if all deviations are below threshold
  float threshold = _supersamplingParams.adaptiveDistance;
  if (d00 < threshold && d01 < threshold && d10 < threshold && d11 < threshold)
  {
    return average;
  }
  
  // Subdivide into four subpixels
  float cx = (x0 + x1) * 0.5f;
  float cy = (y0 + y1) * 0.5f;
  
  // Sample center point (shared by all four subpixels)
  Color centerColor = getRayColor(cx, cy, true, subpixelSize * 0.5f);
  
  // Recursively sample each subpixel
  // Top-left
  Color c0 = adaptiveSupersample(x0, y0, cx, cy, level + 1);
  // Top-right
  Color c1 = adaptiveSupersample(cx, y0, x1, cy, level + 1);
  // Bottom-left
  Color c2 = adaptiveSupersample(x0, cy, cx, y1, level + 1);
  // Bottom-right
  Color c3 = adaptiveSupersample(cx, cy, x1, y1, level + 1);
  
  return (c0 + c1 + c2 + c3) * 0.25f;
}

Color
RayTracer::trace(const Ray3f& ray, uint32_t level, float weight, float currentIOR)
//[]---------------------------------------------------[]
//|  Trace a ray                                        |
//|  @param the ray                                     |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param currentIOR - index of refraction of current medium |
//|  @return color of the ray                           |
//[]---------------------------------------------------[]
{
  if (level > _maxRecursionLevel)
    return Color::black;
  ++_numberOfRays;

  Intersection hit;

  return intersect(ray, hit) ? shade(ray, hit, level, weight, currentIOR) : background();
}

inline constexpr auto
rt_eps()
{
  return 1e-4f;
}

bool
RayTracer::intersect(const Ray3f& ray, Intersection& hit)
//[]---------------------------------------------------[]
//|  Ray/object intersection                            |
//|  @param the ray (input)                             |
//|  @param information on intersection (output)        |
//|  @return true if the ray intersects an object       |
//[]---------------------------------------------------[]
{
  hit.object = nullptr;
  hit.distance = ray.tMax;
  return _bvh->intersect(ray, hit) ? ++_numberOfHits : false;
}

inline auto
maxRGB(const Color& c)
{
  return math::max(math::max(c.r, c.g), c.b);
}

Color
RayTracer::shade(const Ray3f& ray,
  Intersection& hit,
  uint32_t level,
  float weight,
  float currentIOR)
//[]---------------------------------------------------[]
//|  Shade a point P                                    |
//|  @param the ray (input)                             |
//|  @param information on intersection (input)         |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param currentIOR - index of refraction of current medium |
//|  @return color at point P                           |
//[]---------------------------------------------------[]
{
  auto primitive = (Primitive*)hit.object;

  assert(nullptr != primitive);

  auto N = primitive->normal(hit);
  const auto& V = ray.direction;
  auto NV = N.dot(V);

  // Determine if ray is entering or leaving the object
  bool entering = NV < 0;
  
  // Make sure "real" normal points against ray direction for shading
  vec3f shadingNormal = N;
  if (!entering)
  {
    shadingNormal = -N;
    NV = -NV;
  }

  auto R = V - (2 * NV) * shadingNormal; // reflection vector
  // Start with ambient lighting
  auto m = primitive->material();
  auto color = _scene->ambientLight * m->ambient;
  auto P = ray(hit.distance);

  // Compute direct lighting
  for (auto light : _scene->lights())
  {
    // If the light is turned off, then continue
    if (!light->isTurnedOn())
      continue;

    vec3f L;
    float d;

    // If the point P is out of the light range (for finite
    // point light or spotlight), then continue
    if (!light->lightVector(P, L, d))
      continue;

    auto NL = shadingNormal.dot(L);

    // If light vector is backfaced, then continue
    if (NL <= 0)
      continue;

    auto lightRay = Ray3f{P + L * rt_eps(), L};

    lightRay.tMax = d;
    ++_numberOfRays;
    // If the point P is shadowed, then continue
    if (shadow(lightRay))
      continue;

    auto lc = light->lightColor(d);

    color += lc * m->diffuse * NL;
    if (m->shine <= 0 || (d = R.dot(L)) <= 0)
      continue;
    color += lc * m->spot * pow(d, m->shine);
  }
  
  // Compute specular reflection
  if (m->specular != Color::black)
  {
    auto reflectionWeight = weight * maxRGB(m->specular);
    if (reflectionWeight > _minWeight && level < _maxRecursionLevel)
    {
      auto reflectionRay = Ray3f{P + R * rt_eps(), R};
      color += m->specular * trace(reflectionRay, level + 1, reflectionWeight, currentIOR);
    }
  }
  
  // Compute specular refraction (A1)
  const auto& Ot = m->transparency;
  if (Ot != Color::black) // Material is transparent
  {
    auto nextIOR = m->ior;
    
    // Determine IOR ratio: η12 = η1/η2
    // η1 is current medium IOR, η2 is object material IOR
    float eta12 = entering ? currentIOR / nextIOR : nextIOR / currentIOR;
    
    // L is the incident direction (ray direction V)
    const vec3f& L = V;
    
    // Use the original normal (not shading normal) for refraction calculation
    vec3f refractionNormal = entering ? N : -N;
    
    // C1 = cos(theta1) = -L · N
    float C1 = -L.dot(refractionNormal);
    
    // C2 = cos(theta2) = sqrt(1 - η12²(1 - C1²))
    float C2_squared = 1.0f - eta12 * eta12 * (1.0f - C1 * C1);
    
    if (C2_squared >= 0.0f) // No total internal reflection
    {
      float C2 = std::sqrt(C2_squared);
      
      // Compute refraction direction: T = η12*L + (η12*C1 - C2)*N
      vec3f T = eta12 * L + (eta12 * C1 - C2) * refractionNormal;
      T.normalize();
      
      auto refractionWeight = weight * maxRGB(Ot);
      if (refractionWeight > _minWeight && level < _maxRecursionLevel)
      {
        auto refractionRay = Ray3f{P + T * rt_eps(), T};
        // When ray enters object, use object's IOR; when leaving, use scene IOR (1.0)
        float newIOR = entering ? nextIOR : 1.0f;
        Color Ct = trace(refractionRay, level + 1, refractionWeight, newIOR);
        color += Ot * Ct;
      }
    }
    // If C2_squared < 0, total internal reflection occurs (handled by specular reflection above)
  }
  
  return color;
}

Color
RayTracer::background() const
//[]---------------------------------------------------[]
//|  Background                                         |
//|  @return background color                           |
//[]---------------------------------------------------[]
{
  return _scene->backgroundColor;
}

bool
RayTracer::shadow(const Ray3f& ray)
//[]---------------------------------------------------[]
//|  Verify if ray is a shadow ray                      |
//|  @param the ray (input)                             |
//|  @return true if the ray intersects an opaque object |
//[]---------------------------------------------------[]
{
  Intersection hit;
  hit.object = nullptr;
  hit.distance = ray.tMax;
  
  if (!_bvh->intersect(ray, hit))
    return false;
  
  ++_numberOfHits;
  
  // Check if the intersected object is opaque
  auto primitive = (Primitive*)hit.object;
  if (primitive != nullptr)
  {
    auto material = primitive->material();
    if (material != nullptr)
    {
      // Material is opaque if transparency color is black
      if (material->transparency != Color::black)
      {
        // Transparent object - light passes through (eventually, without color/direction change)
        // For now, we consider transparent objects as not blocking light
        return false;
      }
    }
  }
  
  // Opaque object blocks the light
  return true;
}

} // end namespace cg
