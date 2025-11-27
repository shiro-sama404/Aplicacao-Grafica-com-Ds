//[]---------------------------------------------------------------[]
//|                                                                 |
//| PBRActor.h                                                      |
//|                                                                 |
//| PBR Actor with shape and material for TP1                      |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Shape3.h"
#include "PBRMaterial.h"
#include "core/SharedObject.h"
#include "math/Matrix4x4.h"
#include <string>

namespace cg
{ // begin namespace cg

//
// PBRActor: ator com forma geométrica e material PBR
//
class PBRActor: public SharedObject
{
public:
  bool visible;

  // Construtor com nome, shape e material
  PBRActor(const std::string& actorName, 
           Shape3* shape,
           PBRMaterial* material):
    _name{actorName},
    _shape{shape},
    _material{material},
    _transform{mat4f::identity()},
    _inverse{mat4f::identity()},
    _normalMatrix{mat3f::identity()},
    visible{true}
  {
    // do nothing
  }

  // Nome do ator
  const char* name() const
  {
    return _name.c_str();
  }

  void setName(const std::string& actorName)
  {
    _name = actorName;
  }

  // Visibilidade
  bool isVisible() const
  {
    return visible;
  }

  void setVisible(bool v)
  {
    visible = v;
  }

  // Shape (forma geométrica)
  Shape3* shape() const
  {
    return _shape;
  }

  // Transformação
  const mat4f& transform() const
  {
    return _transform;
  }

  void setTransform(const mat4f& transform)
  {
    _transform = transform;
    
    // Calcular inversa para normal matrix
    if (!_transform.inverse(_inverse, math::Limits<float>::eps()))
      _inverse = mat4f::identity();
    
    // Normal matrix = (M^-1)^T
    _normalMatrix = mat3f{_inverse}.transpose();
  }

  const mat4f& inverseTransform() const
  {
    return _inverse;
  }

  const mat3f& normalMatrix() const
  {
    return _normalMatrix;
  }

  // Material PBR (com Reference)
  const PBRMaterial* pbrMaterial() const
  {
    return _material;
  }

  PBRMaterial* pbrMaterial()
  {
    return _material;
  }

  void setPBRMaterial(PBRMaterial* material)
  {
    _material = material;
  }

  // Métodos de conveniência para modificar material
  void setDiffuseColor(const Color& color)
  {
    if (_material)
      _material->Od = color;
  }

  void setSpecularColor(const Color& color)
  {
    if (_material)
      _material->Os = color;
  }

  void setRoughness(float roughness)
  {
    if (_material)
      _material->roughness = math::clamp(roughness, 0.0f, 1.0f);
  }

  void setMetalness(float metalness)
  {
    if (_material)
      _material->metalness = math::clamp(metalness, 0.0f, 1.0f);
  }

private:
  std::string _name;
  Reference<Shape3> _shape;
  Reference<PBRMaterial> _material;
  mat4f _transform;
  mat4f _inverse;
  mat3f _normalMatrix;

}; // PBRActor

} // end namespace cg