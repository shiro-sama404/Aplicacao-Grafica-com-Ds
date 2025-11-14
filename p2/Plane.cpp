#include "Plane.h"

bool Plane::intersect(const Ray3<float>& ray, float& t) const
{
    constexpr float EPS = 1e-6f;

    float denom = _normal.dot(ray.direction);

    if (std::fabs(denom) < EPS)
        return false;

    t = -(_normal.dot(ray.origin) + _d) / denom;

    if (t < EPS)
        return false;

    vec3 p = ray.origin + ray.direction * t;

    if (fabs(p.x) > 1.0f || fabs(p.z) > 1.0f) 
        return false;

    return true;
}

vec3 Plane::normalAt(const vec3& p) const
{
    return _normal;
}

Bounds3f Plane::bounds() const
{
    //Quadrado 2x2 no plano XZ, com uma espessura mínima
    return Bounds3f{ vec3f{ -1.0f, -1e-4f, -1.0f }, vec3f{ 1.0f, 1e-4f, 1.0f } };
}