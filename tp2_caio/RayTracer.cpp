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
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

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
  return ((float)rand() / RAND_MAX / 4.0f) - 0.125f;
}

inline auto
maxRGB(const Color& c)
{
  return math::max(math::max(c.r, c.g), c.b);
}

inline constexpr auto
rt_eps()
{
  return 1e-4f;
}

} // end namespace


/////////////////////////////////////////////////////////////////////
//
// RayTracer implementation
// =========
RayTracer::RayTracer(SceneBase& scene, Camera& camera):
  Renderer{scene, camera},
  _maxRecursionLevel{6},
  _minWeight{minMinWeight},
  _adaptiveThreshold{0.1f},
  _maxSubdivisionLevel{2},
  _useJitter{false},
  _sceneIOR{1.0f}
{
  // do nothing
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
  _bvh = new PrimitiveBVH{move(primitives)};
}

void
RayTracer::render()
{
  throw runtime_error("RayTracer::render() invoked");
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
  scan(image);

  auto et = timer.time();

  cout << "\nNumber of rays: " << _numberOfRays;
  cout << "\nNumber of hits: " << _numberOfHits;
  printElapsedTime("\nDONE! ", et);
}

void
RayTracer::setPixelRay(float x, float y)
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
  int w = _viewport.w;
  int h = _viewport.h;
  ImageBuffer scanLine{w, 1};

  // Standard non-adaptive scan
  if (_maxSubdivisionLevel == 0)
  {
    for (auto j = 0; j < h; j++)
    {
      auto y = (float)j + 0.5f;

      printf("Scanning line %d of %d\r", j + 1, h);
      for (auto i = 0; i < w; i++)
        scanLine[i] = shoot((float)i + 0.5f, y);
      image.setData(0, j, scanLine);
    }
    return;
  }

  // Adaptive Supersampling
  const int steps = 1 << _maxSubdivisionLevel;
  _lineBuffer.resize(w * steps + 1);
  
  // Initialize line buffer as Raw
  for (auto& p : _lineBuffer) 
    p.cooked = false;

  for (auto j = 0; j < h; j++)
  {
    printf("Scanning line %d of %d\r", j + 1, h);

    // Reset Left edge of the window to Raw at start of line
    for(int wy = 0; wy <= steps; ++wy)
      _window[wy][0].cooked = false;

    for (auto i = 0; i < w; i++)
    {
      for (int wx = 0; wx <= steps; ++wx)
      {
        int bufferIndex = i * steps + wx;
        if (bufferIndex < (int)_lineBuffer.size())
          _window[0][wx] = _lineBuffer[bufferIndex];
      }

      for (int wy = 1; wy <= steps; ++wy)
        for (int wx = 1; wx <= steps; ++wx)
          _window[wy][wx].cooked = false;

      // Compute pixel color using adaptive function
      scanLine[i] = adapt(0, 0, steps, (float)i, (float)j);

      for (int wx = 0; wx <= steps; ++wx)
      {
        int bufferIndex = i * steps + wx;
        if (bufferIndex < (int)_lineBuffer.size())
          _lineBuffer[bufferIndex] = _window[steps][wx];
      }

      for (int wy = 0; wy <= steps; ++wy)
        _window[wy][0] = _window[wy][steps];
    }
    image.setData(0, j, scanLine);
  }
}

Color
RayTracer::adapt(int i, int j, int step, float x, float y)
//[]---------------------------------------------------[]
//|  Adaptative recursive sampling                        |
//|  @param i, j: top-left index in the sliding window  |
//|  @param step: current step size in window indices   |
//|  @param x, y: origin coordinates of the pixel       |
//[]---------------------------------------------------[]
{
  // Define corners indices in window
  int coords[4][2] = {
    {i, j}, 
    {i + step, j}, 
    {i, j + step}, 
    {i + step, j + step}
  };

  Color colors[4];

  // Max steps used for normalization
  float invMaxSteps = 1.0f / (float)(1 << _maxSubdivisionLevel);

  for (int k = 0; k < 4; ++k)
  {
    int wi = coords[k][0];
    int wj = coords[k][1];
    
    GridPoint& p = _window[wj][wi];

    if (!p.cooked)
    {
      float offsetX = (float)wi * invMaxSteps;
      float offsetY = (float)wj * invMaxSteps;

      // Apply Jitter if needed.
      float jx = _useJitter ? arand() : 0.0f;
      float jy = _useJitter ? arand() : 0.0f;

      p.color = shoot(x + offsetX + jx, y + offsetY + jy);
      p.cooked = true;
    }
    colors[k] = p.color;
  }

  Color avg = (colors[0] + colors[1] + colors[2] + colors[3]) * 0.25f;

  if (step <= 1)
    return avg;

  // Check threshold
  bool subdivide = false;
  for (int k = 0; k < 4; ++k)
  {
    Color diff = colors[k] - avg;
    if (maxRGB(Color{abs(diff.r), abs(diff.g), abs(diff.b)}) > _adaptiveThreshold)
    {
      subdivide = true;
      break;
    }
  }

  if (subdivide)
  {
    int newStep = step / 2;
    Color c1 = adapt(i, j, newStep, x, y);
    Color c2 = adapt(i + newStep, j, newStep, x, y);
    Color c3 = adapt(i, j + newStep, newStep, x, y);
    Color c4 = adapt(i + newStep, j + newStep, newStep, x, y);
    
    return (c1 + c2 + c3 + c4) * 0.25f;
  }

  return avg;
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

  // Initialize IOR stack with scene IOR
  vector<float> iorStack;
  iorStack.reserve(8);
  iorStack.push_back(_sceneIOR);

  // trace pixel ray
  Color color = trace(_pixelRay, 0, 1, iorStack);

  // adjust RGB color
  if (color.r > 1.0f) color.r = 1.0f;
  if (color.g > 1.0f) color.g = 1.0f;
  if (color.b > 1.0f) color.b = 1.0f;
  
  return color;
}

Color
RayTracer::trace(const Ray3f& ray, uint32_t level, float weight, const vector<float>& iorStack)
//[]---------------------------------------------------[]
//|  Trace a ray                                        |
//|  @param the ray                                     |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param ior stack                                   |
//|  @return color of the ray                           |
//[]---------------------------------------------------[]
{
  if (level > _maxRecursionLevel)
    return Color::black;
  ++_numberOfRays;

  Intersection hit;

  return intersect(ray, hit) ? shade(ray, hit, level, weight, iorStack) : background();
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

Color
RayTracer::shade(const Ray3f& ray,
  Intersection& hit,
  uint32_t level,
  float weight,
  const vector<float>& iorStack)
//[]---------------------------------------------------[]
//|  Shade a point P                                    |
//|  @param the ray (input)                             |
//|  @param information on intersection (input)         |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param ior stack                                   |
//|  @return color at point P                           |
//[]---------------------------------------------------[]
{
  auto primitive = (Primitive*)hit.object;

  assert(nullptr != primitive);

  auto N = primitive->normal(hit);
  const auto& V = ray.direction;
  auto NV = N.dot(V);

  // Determine entering/leaving
  bool entering = NV < 0; 

  if (!entering)
  {
      N.negate();
      NV = -NV; 
  }

  auto R = V - (2 * NV) * N; // reflection vector
  R.normalize();
  
  // Start with ambient lighting
  auto m = primitive->material();
  auto color = _scene->ambientLight * m->ambient;
  auto P = ray(hit.distance);

  // Compute direct lighting
  for (auto light : _scene->lights())
  {
    // If the light is turned off, then continue
    if (!light->isTurnedOn()) continue;

    vec3f L;
    float d;

    // If the point P is out of the light range (for finite
    // point light or spotlight), then continue
    if (!light->lightVector(P, L, d)) continue;

    auto NL = N.dot(L);
    // If light vector is backfaced, then continue
    if (NL <= 0) continue;

    auto lightRay = Ray3f{P + L * rt_eps(), L};
    lightRay.tMax = d;
    ++_numberOfRays;
    
    // If the point P is shadowed, then continue
    if (shadow(lightRay)) continue;

    auto lc = light->lightColor(d);
    color += lc * m->diffuse * NL;
    
    if (m->shine > 0)
    {
      auto RL = R.dot(L);
      if (RL > 0)
        color += lc * m->spot * pow(RL, m->shine);
    }
  }
  
  // Compute specular reflection
  if (m->specular != Color::black)
  {
    float w = weight * maxRGB(m->specular);
    if (w > _minWeight && level < _maxRecursionLevel)
    {
      auto reflectionRay = Ray3f{P + R * rt_eps(), R};
      color += m->specular * trace(reflectionRay, level + 1, w, iorStack);
    }
  }
  
  // Refraction
  if (m->transparency != Color::black)
  {
    float n1 = iorStack.empty() ? 1.0f : iorStack.back();
    float n2 = m->ior;
    
    if (!entering)
    {
      n1 = m->ior;
      bool found = false;
      for (auto it = iorStack.rbegin(); it != iorStack.rend(); ++it)
      {
        if (abs(*it - m->ior) < 1e-5f)
        {
          auto nextIt = next(it);
          if (nextIt != iorStack.rend())
            n2 = *nextIt;
          else
            n2 = _sceneIOR;
          found = true;
          break;
        }
      }
      if (!found) n2 = _sceneIOR;
    }
    
    float eta = n1 / n2;
    float C1 = -NV;
    float discriminant = 1.0f - eta * eta * (1.0f - C1 * C1);
    
    if (discriminant >= 0.0f)
    {
      vec3f T = eta * V + (eta * C1 - sqrt(discriminant)) * N;
      T.normalize();
      
      float w = weight * maxRGB(m->transparency);
      if (w > _minWeight && level < _maxRecursionLevel)
      {
        vector<float> nextStack = iorStack; 
        
        if (entering)
          nextStack.push_back(m->ior);
        else
          for (auto it = nextStack.rbegin(); it != nextStack.rend(); ++it)
            if (abs(*it - m->ior) < 1e-5f)
            {
              nextStack.erase(next(it).base());
              break;
            }

        auto refractionRay = Ray3f{P + T * rt_eps(), T};
        color += m->transparency * trace(refractionRay, level + 1, w, nextStack);
      }
    }
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
//|  Verifiy if ray is a shadow ray                     |
//|  @param the ray (input)                             |
//|  @return true if the ray intersects an object       |
//[]---------------------------------------------------[]
{
  Ray3f currentRay = ray;
  Intersection hit;
  
  while (true)
  {
    hit.object = nullptr;
    hit.distance = currentRay.tMax;
    
    if (!_bvh->intersect(currentRay, hit))
      break;
    
    auto primitive = (Primitive*)hit.object;
    if (primitive != nullptr)
    {
      auto m = primitive->material();
      if (m->transparency == Color::black)
      {
        ++_numberOfHits;
        return true;
      }
      float newTMin = hit.distance + rt_eps();
      if (newTMin >= currentRay.tMax)
        break;
      currentRay.tMin = newTMin;
    }
  }
  
  return false;
}

} // end namespace cg