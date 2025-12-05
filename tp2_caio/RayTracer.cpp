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
  if (_maxSubdivisionLevel == 0)
  {
    // No antialiasing - use original method
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
  else
  {
    // Adaptive supersampling
    // Buffer to store ray colors: (width+1) x (height+1) for all corners
    std::vector<std::vector<Color>> rayBuffer(_viewport.h + 1, std::vector<Color>(_viewport.w + 1));
    
    // Initialize buffer with invalid color (to mark untraced rays)
    for (auto& row : rayBuffer)
      for (auto& col : row)
        col = Color{-1.0f, -1.0f, -1.0f};
    
    ImageBuffer scanLine{_viewport.w, 1};

    for (auto j = 0; j < _viewport.h; j++)
    {
      printf("Scanning line %d of %d\r", j + 1, _viewport.h);
      for (auto i = 0; i < _viewport.w; i++)
        scanLine[i] = samplePixel((float)i, (float)j, rayBuffer);
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

  // Initialize IOR stack with scene IOR
  std::vector<float> iorStack;
  iorStack.reserve(8); // Optimization: Pre-allocate to avoid reallocs
  iorStack.push_back(_sceneIOR);

  // trace pixel ray
  Color color = trace(_pixelRay, 0, 1, iorStack);

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
RayTracer::trace(const Ray3f& ray, uint32_t level, float weight, const std::vector<float>& iorStack)
//[]---------------------------------------------------[]
//|  Trace a ray                                        |
//|  @param the ray                                     |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param iorStack - stack of IORs (passed by ref)    |
//|  @return color of the ray                           |
//[]---------------------------------------------------[]
{
  if (level > _maxRecursionLevel)
    return Color::black;
  ++_numberOfRays;

  Intersection hit;

  return intersect(ray, hit) ? shade(ray, hit, level, weight, iorStack) : background();
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
  const std::vector<float>& iorStack)
//[]---------------------------------------------------[]
//|  Shade a point P                                    |
//|  @param the ray (input)                             |
//|  @param information on intersection (input)         |
//|  @param recursion level                             |
//|  @param ray weight                                  |
//|  @param iorStack - stack of IORs (passed by ref)    |
//|  @return color at point P                           |
//[]---------------------------------------------------[]
{
  auto primitive = (Primitive*)hit.object;

  assert(nullptr != primitive);

  auto N = primitive->normal(hit);
  const auto& V = ray.direction;
  auto NV = N.dot(V);

  // [IMPORTANT] Determine entering/leaving BEFORE flipping normal
  bool entering = NV < 0; 

  // Make sure "real" normal is on right side (para Phong/Reflexão)
  if (!entering) // Se NV > 0, estamos saindo (ou normal invertida)
  {
      N.negate();
      NV = -NV; 
  }

  // Calculate reflection vector: R = V - 2 * (N . V) * N
  auto R = V - (2 * NV) * N; // reflection vector
  R.normalize(); // Ensure R is normalized
  
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

    auto NL = N.dot(L);

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
    // Calculate specular highlight using reflection vector
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
      
      // [OPTIMIZATION] Reflection doesn't change medium.
      // Pass original 'iorStack' by const reference. Cost: Zero allocation.
      color += m->specular * trace(reflectionRay, level + 1, w, iorStack);
    }
  }
  
  // Compute refraction (transparency)
  if (m->transparency != Color::black)
  {
    // Identify IORs for Snell's Law
    // n1 = current medium (top of stack)
    float n1 = iorStack.empty() ? 1.0f : iorStack.back();
    float n2 = m->ior; // Assume we are entering the object
    
    // If we are exiting, we need to find out where we are going (new top after removing this)
    if (!entering)
    {
      // Logic to "look ahead" without modifying the stack yet
      n1 = m->ior; // We are currently in n1
      
      // Reverse search to find the previous IOR
      bool found = false;
      for (auto it = iorStack.rbegin(); it != iorStack.rend(); ++it)
      {
        // Use epsilon for float comparison
        if (std::abs(*it - m->ior) < 1e-5f)
        {
          // The element *before* this in the stack will be the new medium n2
          auto nextIt = std::next(it);
          if (nextIt != iorStack.rend())
            n2 = *nextIt;
          else
            n2 = _sceneIOR; // Stack would become empty
          found = true;
          break;
        }
      }
      if (!found) n2 = _sceneIOR; // Fallback
    }
    
    float eta = n1 / n2;
    float C1 = -NV; // cos(theta1)
    float discriminant = 1.0f - eta * eta * (1.0f - C1 * C1);
    
    // Check if total internal reflection occurs
    if (discriminant >= 0.0f)
    {
      // Calculate refraction direction
      vec3f T = eta * V + (eta * C1 - std::sqrt(discriminant)) * N;
      T.normalize();
      
      float w = weight * maxRGB(m->transparency);
      if (w > _minWeight && level < _maxRecursionLevel)
      {
        // [COPY-ON-WRITE] Only here we pay the price of copying the stack
        std::vector<float> nextStack = iorStack; 
        
        if (entering)
        {
          nextStack.push_back(m->ior);
        }
        else
        {
          // Advanced removal logic: Find specific IOR from back and remove
          for (auto it = nextStack.rbegin(); it != nextStack.rend(); ++it)
          {
            if (std::abs(*it - m->ior) < 1e-5f)
            {
              // std::next(it).base() converts reverse_iterator to iterator pointing to the element
              nextStack.erase(std::next(it).base());
              break;
            }
          }
        }

        auto refractionRay = Ray3f{P + T * rt_eps(), T};
        color += m->transparency * trace(refractionRay, level + 1, w, nextStack);
      }
    }
    // If discriminant < 0, Total Internal Reflection occurs (no refraction contribution added)
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
//|  @return true if the ray intersects an opaque object |
//[]---------------------------------------------------[]
{
  Ray3f currentRay = ray;
  Intersection hit;
  
  // Check for intersections along the ray path
  while (true)
  {
    hit.object = nullptr;
    hit.distance = currentRay.tMax;
    
    if (!_bvh->intersect(currentRay, hit))
      break; // No more intersections
    
    auto primitive = (Primitive*)hit.object;
    if (primitive != nullptr)
    {
      auto m = primitive->material();
      // If material is opaque (transparency is black), shadow is cast
      if (m->transparency == Color::black)
      {
        ++_numberOfHits;
        return true;
      }
      // If transparent, continue the ray through the object
      float newTMin = hit.distance + rt_eps();
      if (newTMin >= currentRay.tMax)
        break; // Past the end of the ray
      currentRay.tMin = newTMin;
    }
  }
  
  return false;
}

inline auto
arand()
{
  return ((float)std::rand() / RAND_MAX / 4.0f) - 0.125f;
}

Color
RayTracer::samplePixel(float x, float y, std::vector<std::vector<Color>>& rayBuffer)
{
  // Buffer optimization for Level 0 corners
  Color corners[4];
  int ix = (int)x;
  int iy = (int)y;
  float offsets[4][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
  
  for (int i = 0; i < 4; i++)
  {
    int cx = ix + (int)offsets[i][0];
    int cy = iy + (int)offsets[i][1];
    
    // Check if ray already traced in buffer
    if (cx <= _viewport.w && cy <= _viewport.h && 
        rayBuffer[cy][cx].r >= 0.0f)
    {
      corners[i] = rayBuffer[cy][cx];
    }
    else
    {
      float jitterX = _useJitter ? arand() : 0.0f;
      float jitterY = _useJitter ? arand() : 0.0f;
      float px = (float)cx + jitterX;
      float py = (float)cy + jitterY;
      
      setPixelRay(px, py);
      
      // Create fresh stack for primary ray
      std::vector<float> stack;
      stack.reserve(8);
      stack.push_back(_sceneIOR);
      
      corners[i] = trace(_pixelRay, 0, 1.0f, stack);
      
      // Store in buffer
      if (cx <= _viewport.w && cy <= _viewport.h)
        rayBuffer[cy][cx] = corners[i];
    }
  }
  
  // Adaptive recursion starts here
  return sampleSubpixel(x, y, x + 1.0f, y + 1.0f, 0);
}

Color
RayTracer::sampleSubpixel(float x0, float y0, float x1, float y1, int level)
{
  // Re-calculate or pass corners? 
  // For simplicity and correctness in recursion, we re-shoot or calculate variance 
  // based on the 4 corners of the current rect (x0,y0) to (x1,y1).
  
  float jitterX = _useJitter ? arand() : 0.0f;
  float jitterY = _useJitter ? arand() : 0.0f;
  
  // Define corners of this sub-rectangle
  float coords[4][2] = {{x0,y0}, {x1,y0}, {x0,y1}, {x1,y1}};
  Color colors[4];
  Color avg = Color::black;
  
  for(int i=0; i<4; ++i)
  {
    setPixelRay(coords[i][0] + jitterX, coords[i][1] + jitterY);
    
    std::vector<float> stack;
    stack.reserve(8);
    stack.push_back(_sceneIOR);
    
    colors[i] = trace(_pixelRay, 0, 1.0f, stack);
    avg += colors[i];
  }
  avg *= 0.25f;

  // Calculate deviation
  float maxDev = 0.0f;
  for(int i=0; i<4; ++i)
  {
    Color diff = colors[i] - avg;
    float dev = std::max(std::max(std::abs(diff.r), std::abs(diff.g)), std::abs(diff.b));
    maxDev = std::max(maxDev, dev);
  }

  // Adaptive check
  if (maxDev < _adaptiveThreshold || level >= (int)_maxSubdivisionLevel)
  {
    return avg;
  }

  // Subdivide
  float xm = (x0 + x1) * 0.5f;
  float ym = (y0 + y1) * 0.5f;
  
  Color c1 = sampleSubpixel(x0, y0, xm, ym, level + 1); // Top-Left
  Color c2 = sampleSubpixel(xm, y0, x1, ym, level + 1); // Top-Right
  Color c3 = sampleSubpixel(x0, ym, xm, y1, level + 1); // Bottom-Left
  Color c4 = sampleSubpixel(xm, ym, x1, y1, level + 1); // Bottom-Right
  
  return (c1 + c2 + c3 + c4) * 0.25f;
}

} // end namespace cg