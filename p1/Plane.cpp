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

    if (fabs(p.x) > 10.0f || fabs(p.z) > 10.0f)
        return false;

    return true;
}

vec3 Plane::normalAt(const vec3& p) const
{
    return _normal;
}