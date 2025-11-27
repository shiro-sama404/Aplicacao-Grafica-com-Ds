//[]---------------------------------------------------------------[]
//|                                                                 |
//| PBRActor.h                                                      |
//|                                                                 |
//| PBR Actor with shape and material for TP1                       |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"
#include "PBRMaterial.h"
#include "geometry/Bounds3.h"
#include "geometry/Ray.h"
#include "geometry/Intersection.h"

#include <math/matrix4x4.h>
#include <string>

namespace cg
{ // begin namespace cg
//
// PBRActor: ator com forma geométrica e material PBR
//
class PBRActor : public SharedObject
{
using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

public:
  bool visible;

  // Construtor com nome, shape e material
  PBRActor
  (
    const std::string& actorName, 
    Shape3* shape,
    const PBRMaterial& material
  ):
    _name{actorName},
    _shape{shape},
    _material{material},
    _transform{ mat4::identity() },
    _inverse{ mat4::identity() },
    _normalMatrix{ mat3::identity()},
    visible{true}
  {}

  // Nome do ator
  const char* name() const { return _name.c_str(); }
  void setName(const std::string& actorName){  _name = actorName; }

  // Visibilidade
  bool isVisible() const { return visible; }
  void setVisible(bool v){ visible = v; }

  // Shape (forma geométrica)
  Shape3* shape() const { return _shape; }

  // Transformação
  const mat4f transform() const { return _transform; }
  void setTransform(const mat4f& transform)
  {
    _transform = transform;

    if (!_transform.inverse(_inverse, cg::math::Limits<float>::eps()))
      _inverse = mat4::identity();

    _normalMatrix = (mat3(_inverse)).transpose();
  }

  const mat4& inverseTransform() const { return _inverse; }
  const mat3& normalMatrix() const { return _normalMatrix; }

  // Material PBR
  const PBRMaterial& pbrMaterial() const { return _material; }
  PBRMaterial& pbrMaterial() { return _material; }
  void setPBRMaterial(const PBRMaterial& material) { _material = material; }

  // Métodos para BVH
  Bounds3f bounds() const
  {
    if (_shape == nullptr)
      return Bounds3f{};
    
    // Transformar bounds do shape para espaço global
    Bounds3f localBounds = _shape->bounds();
    Bounds3f globalBounds{localBounds, _transform};
    
    return globalBounds;
  }

  bool intersect(const Ray3f& ray) const
  {
    if (_shape == nullptr)
      return false;
    
    // Transformar raio para espaço local
    const auto& invTransform = _inverse;
    Ray3f localRay;
    localRay.origin = invTransform.transform3x4(ray.origin);
    localRay.direction = invTransform.transformVector(ray.direction).versor();
    localRay.tMin = ray.tMin;
    localRay.tMax = ray.tMax;
    
    float distance = ray.tMax;
    return _shape->intersect(localRay, distance);
  }

  bool intersect(const Ray3f& ray, Intersection& hit) const
  {
    if (_shape == nullptr)
      return false;
    
    // Transformar raio para espaço local
    const auto& invTransform = _inverse;
    Ray3f localRay;
    localRay.origin = invTransform.transform3x4(ray.origin);
    localRay.direction = invTransform.transformVector(ray.direction).versor();
    localRay.tMin = ray.tMin;
    localRay.tMax = hit.distance;
    
    float localDistance = hit.distance;
    if (_shape->intersect(localRay, localDistance))
    {
      // Transformar ponto de interseção de volta para espaço global
      vec3f localPoint = localRay(localDistance);
      vec3f worldPoint = _transform.transform3x4(localPoint);
      vec3f toPoint = worldPoint - ray.origin;
      float worldDistance = toPoint.length();
      
      // Verificar se o raio está indo na direção correta
      if (toPoint.dot(ray.direction) < 0)
        return false;
      
      if (worldDistance < hit.distance && worldDistance > ray.tMin)
      {
        hit.object = (void*)this;
        hit.distance = worldDistance;
        return true;
      }
    }
    
    return false;
  }

private:
  std::string _name;
  Reference<Shape3> _shape;
  PBRMaterial _material;
  mat4 _transform;
  mat4 _inverse;
  mat3 _normalMatrix;
}; // PBRActor

} // end namespace cg