#pragma once
#include <math/Vector3.h>
#include "PBRActor.h"

using vec3 = Vector3<float>;

struct Intersection
{
  const PBRActor* actor = nullptr;
  float distance = 0;
  vec3 point;
  vec3 normal;
};