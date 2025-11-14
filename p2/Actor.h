#pragma once
#include <graphics/Material.h>
#include <math/matrix4x4.h>
#include <geometry/Bounds3.h>
#include <geometry/Intersection.h> 
#include "Shape3.h"

using namespace cg;
using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

class Actor : public SharedObject
{
public:
    Actor(Shape3* shape, Material* material) :
        _shape{ shape }, _material{ material },
        _transform{ mat4::identity() },
        _inverse{ mat4::identity() },
        _normalMatrix{ mat3::identity() } {
    }

    void setTransform(const mat4& m)
    {
        _transform = m;
        if (!_transform.inverse(_inverse, cg::math::Limits<float>::eps()))
            _inverse = mat4::identity();

        _normalMatrix = (mat3(_inverse)).transpose();
    }

    Bounds3f bounds() const
    {
        Bounds3f b = _shape->bounds();
        b.transform(_transform);
        return b;
    }

    //Interseção booleana (para sombras)
    bool intersect(const Ray3f& ray) const;

    //Interseção com struct da biblioteca (cg::Intersection)
    bool intersect(const Ray3f& ray, cg::Intersection& hit) const;


    Shape3* shape() const { return _shape; }
    Material* material() const { return _material; }
    const mat4& transform() const { return _transform; }
    const mat4& inverseTransform() const { return _inverse; }
    const mat3& normalMatrix() const { return _normalMatrix; }

private:
  Reference<Shape3> _shape;
  Reference<Material> _material;
  mat4 _transform;
  mat4 _inverse;
  mat3 _normalMatrix;

};