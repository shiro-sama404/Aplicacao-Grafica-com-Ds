#pragma once

#include "Shape3.h"
#include "PBRMaterial.h"
#include "geometry/Bounds3.h"
#include "geometry/Ray.h"
#include "geometry/Intersection.h"

#include <math/matrix4x4.h>
#include <string>

namespace cg
{

// Entidade principal da cena, compondo uma forma geométrica (Shape) e um material (PBRMaterial).
// Responsável por gerenciar transformações espaciais (Mundo <-> Objeto) e testes de interseção.
class PBRActor : public TransformableObject
{
using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

public:
  // Construtor: Inicializa o ator com geometria, material e matrizes de transformação identidade.
  PBRActor
  (
    const std::string& actorName, 
    Shape3* shape,
    PBRMaterial* material
  ):
    _name{actorName},
    _shape{shape},
    _material{material},
    _normalMatrix{ mat3::identity()},
    _visible{true}
  {}

  const char* name() const { return _name.c_str(); }
  void setName(const std::string& actorName){  _name = actorName; }

  bool isVisible() const { return _visible; }
  void setVisible(bool v){ _visible = v; }

  // Retorna a primitiva geométrica associada.
  Shape3* shape() const { return _shape; }

  virtual const vec3f& position(){ return _position; }
  virtual void setPosition(const vec3f& pos)
  {
    _position = pos;
    setTransform(pos, quatf::identity(), vec3f{1.0f});
  }
  
  // Acesso e modificação da matriz de transformação (Model Matrix).
  const mat4f transform() const { return localToWorldMatrix(); }
  const mat4& inverseTransform() const { return worldToLocalMatrix(); }
  const mat3& normalMatrix() const { return _normalMatrix; }

  void setTransform(const mat4f& l2w)
  {
    mat4f w2l;
    l2w.inverse(w2l);
    TransformableObject::setTransform(l2w, w2l);
    mat3f normal(l2w);
    normal.invert();
    normal.transpose();
    _normalMatrix = normal;
  }

  void setTransform(const vec3f& position, const quatf& rotation, const vec3f& scale)
  {
    setTransform(mat4f::TRS(position, rotation, scale));
  }

  // Gerenciamento do material.
  const PBRMaterial* pbrMaterial() const { return _material; }
  PBRMaterial* pbrMaterial() { return _material; }
  void setPBRMaterial(PBRMaterial* material) { _material = material; }

  // Calcula a AABB (Axis-Aligned Bounding Box) no espaço do mundo.
  Bounds3f bounds() const
  {
    if (!_shape) return Bounds3f{};
    
    Bounds3f localBounds = _shape->bounds();
    // Aplica a transformação afim à AABB local para obter a AABB global.
    Bounds3f globalBounds{localBounds, transform()};
    
    return globalBounds;
  }

  bool intersect(const Ray3f& ray) const
  {
    Intersection hit;
    return intersect(ray, hit);
  }
  
  bool intersect(const Ray3f& ray, Intersection& hit) const
  {
    if (!_shape || !isVisible())
      return false;
    
    // Transformação do Raio: Mundo -> Local
    const auto& invTransform = inverseTransform();
    Ray3f localRay;
    localRay.origin = invTransform.transform3x4(ray.origin);
    localRay.direction = invTransform.transformVector(ray.direction).versor();
    localRay.tMin = ray.tMin;
    localRay.tMax = hit.distance;
    
    localRay.tMax = ray.tMax;
    float localDistance = ray.tMax;
    
    // Interseção na Primitiva (Espaço Local)
    if (_shape->intersect(localRay, localDistance))
    {
      vec3f localPoint = localRay(localDistance);
      vec3f worldPoint = transform().transform3x4(localPoint);
      vec3f toPoint = worldPoint - ray.origin;
      float worldDistance = toPoint.length();
      
      // Validação de direcionalidade e limites de profundidade.
      if (toPoint.dot(ray.direction) < 0)
        return false;
      
      if (worldDistance < ray.tMax && worldDistance > ray.tMin)
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
  Reference<PBRMaterial> _material;
  mat3 _normalMatrix;
  vec3f _position;
  bool _visible;
};

}