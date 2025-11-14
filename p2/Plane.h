#pragma once
#include "Shape3.h"

class Plane final : public Shape3
{
public:

    Plane() : Shape3{ GLGraphics3::quad() }, _normal{ 0, 1, 0 }, _d{ 0.0f } {

    }


    bool intersect(const Ray3<float>& ray, float& t) const override;
    vec3 normalAt(const vec3& p = vec3(0,0,0)) const override;

    Bounds3f bounds() const override;

private:
    vec3 _normal;
    float _d; //distância do plano à origem (ax + by + cz + d = 0)
};