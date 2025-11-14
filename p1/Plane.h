#pragma once
#include "Shape3.h"

class Plane final : public Shape3
{
public:
    Plane(const vec3& normal = vec3{ 0, 1, 0 }, float d = 0.0f) :
        Shape3{ nullptr }, _normal{ normal.versor() }, _d{ d } {
    }

    bool intersect(const Ray3<float>& ray, float& t) const override;
    vec3 normalAt(const vec3& p = vec3(0,0,0)) const override;

    float getDistance() const { return _d; }

private:
    vec3 _normal;
    float _d; // distância do plano à origem (ax + by + cz + d = 0)
};