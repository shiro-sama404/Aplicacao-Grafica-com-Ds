#pragma once
#include <math/Vector3.h>
#include "Actor.h"

using vec3 = Vector3<float>;

struct Intersection
{
  const Actor* actor = nullptr;
  float distance = 0;
  vec3 point;
  vec3 normal;
};