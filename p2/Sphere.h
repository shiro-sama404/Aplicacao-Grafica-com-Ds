#pragma once
#include "Shape3.h"

class Sphere final : public Shape3
{
public:
    Sphere() : Shape3{ GLGraphics3::sphere() } {
    }

    bool intersect(const Ray3<float>& ray, float& t) const override;
    vec3 normalAt(const vec3& p) const override;

    Bounds3f bounds() const override;
};