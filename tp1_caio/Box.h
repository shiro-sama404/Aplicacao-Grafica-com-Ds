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
// OVERVIEW: Box.h
// ========
// Class definition for box shape (AABB).
//
// Author: Paulo Pagliosa
// Last revision: 20/11/2025

#ifndef __Box_h
#define __Box_h

#include "graphics/Shape.h"
#include "geometry/Bounds3.h"
#include "geometry/Intersection.h"

using namespace cg;

/////////////////////////////////////////////////////////////////////
//
// Box: axis-aligned bounding box shape class
// ===
class Box: public Shape
{
public:
  Box(const vec3f& min, const vec3f& max);
  Box(const Bounds3f& bounds);

  const TriangleMesh* tesselate() const override;
  bool canIntersect() const override;
  vec3f normal(const Intersection&) const override;
  Bounds3f bounds() const override;

private:
  Bounds3f _bounds;

  bool localIntersect(const Ray3f&) const override;
  bool localIntersect(const Ray3f&, Intersection&) const override;

}; // Box

#endif // __Box_h

