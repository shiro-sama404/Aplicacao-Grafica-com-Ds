#include "sphere.h"

bool Sphere::intersect(const Ray3<float>& ray, float& t) const
{
    vec3 oc = ray.origin; //Centro é (0,0,0)
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - 1.0f; //Raio é 1.0
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f)
        return false;

    float sqrtD = std::sqrt(discriminant);
    float t0 = (-b - sqrtD) / (2.0f * a);
    float t1 = (-b + sqrtD) / (2.0f * a);

    if (t0 > 1e-4f)
        t = t0;
    else if (t1 > 1e-4f)
        t = t1;
    else
        return false;

    return true;
}

vec3 Sphere::normalAt(const vec3& p) const
{
    return p.versor(); //Centro é (0,0,0)
}

Bounds3f Sphere::bounds() const
{
    //Limites da esfera unitária local
    return Bounds3f{ vec3f{ -1.0f }, vec3f{ 1.0f } };
}