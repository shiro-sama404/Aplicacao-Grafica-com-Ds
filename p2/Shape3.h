#pragma once
#include <geometry/TriangleMesh.h>
#include <geometry/Ray.h>
#include <graphics/GLGraphics3.h>
#include <graphics/TransformableObject.h>

using namespace cg;

using vec3 = Vector3<float>;

class Shape3 : public TransformableObject
{
public:
    Reference<TriangleMesh> mesh;

    virtual bool intersect(const Ray3<float>& ray, float& t) const = 0;
    virtual vec3 normalAt(const vec3& p) const = 0;
    virtual Bounds3f bounds() const = 0;

protected:
    Shape3(const TriangleMesh* m = nullptr) :
        mesh{m} {}
};