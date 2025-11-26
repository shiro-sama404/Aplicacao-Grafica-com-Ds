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
// OVERVIEW: Box.cpp
// ========
// Source file for box shape.
//
// Author: Paulo Pagliosa
// Last revision: 20/11/2025

#include "Box.h"
#include "graphics/GLGraphics3.h"
#include <cmath>

/////////////////////////////////////////////////////////////////////
//
// Box implementation
// ===
Box::Box(const vec3f& min, const vec3f& max):
  _bounds{min, max}
{
  // do nothing
}

Box::Box(const Bounds3f& bounds):
  _bounds{bounds}
{
  // do nothing
}

const TriangleMesh*
Box::tesselate() const
{
  return GLGraphics3::box();
}

bool
Box::canIntersect() const
{
  return true;
}

bool
Box::localIntersect(const Ray3f& ray) const
{
  float tMin, tMax;
  return _bounds.intersect(ray, tMin, tMax) && tMin <= ray.tMax && tMax >= ray.tMin;
}

bool
Box::localIntersect(const Ray3f& ray, Intersection& hit) const
{
  float tMin, tMax;
  if (!_bounds.intersect(ray, tMin, tMax))
    return false;
  
  if (tMin > ray.tMax || tMax < ray.tMin)
    return false;
  
  // Use the closest intersection point
  float t = tMin >= ray.tMin ? tMin : tMax;
  if (t > ray.tMax)
    return false;
  
  if (t < hit.distance)
  {
    hit.distance = t;
    hit.object = this;
    hit.triangleIndex = -1; // Box doesn't have triangles
    // Store intersection point in p (we'll use it for normal calculation)
    hit.p = ray(t);
    return true;
  }
  return false;
}

vec3f
Box::normal(const Intersection& hit) const
{
  // Calculate normal based on which face was hit
  const vec3f& p = hit.p;
  const vec3f& min = _bounds.min();
  const vec3f& max = _bounds.max();
  
  // Find which face is closest to the intersection point
  // Check each face distance
  float dists[6] = {
    std::abs(p.x - min.x), // -X face
    std::abs(p.x - max.x), // +X face
    std::abs(p.y - min.y), // -Y face
    std::abs(p.y - max.y), // +Y face
    std::abs(p.z - min.z), // -Z face
    std::abs(p.z - max.z)  // +Z face
  };
  
  int minIdx = 0;
  for (int i = 1; i < 6; i++)
  {
    if (dists[i] < dists[minIdx])
      minIdx = i;
  }
  
  // Set normal based on closest face
  vec3f normal{0, 0, 0};
  switch (minIdx)
  {
    case 0: normal = vec3f{-1, 0, 0}; break; // -X
    case 1: normal = vec3f{1, 0, 0}; break;  // +X
    case 2: normal = vec3f{0, -1, 0}; break; // -Y
    case 3: normal = vec3f{0, 1, 0}; break;  // +Y
    case 4: normal = vec3f{0, 0, -1}; break; // -Z
    case 5: normal = vec3f{0, 0, 1}; break;  // +Z
  }
  
  return normal;
}

Bounds3f
Box::bounds() const
{
  return _bounds;
}

