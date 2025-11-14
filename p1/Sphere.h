#pragma once
#include "Shape3.h"

class Sphere final : public Shape3
{
public:
    Sphere(const vec3f& c = vec3f{ 0, 0, 0 }, float r = 1.0f) :
        Shape3{ GLGraphics3::sphere() }, _center{ c }, _radius{ r } {
    }

    bool intersect(const Ray3<float>& ray, float& t) const override;
    vec3 normalAt(const vec3& p) const override;

    vec3f getCenter() const { return _center; }
    float getRadius() const { return _radius; }

private:
    vec3f _center;
    float _radius;
};