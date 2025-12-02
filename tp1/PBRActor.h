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
class PBRActor : public SharedObject
{
using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

public:
  bool visible;

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
    _transform{ mat4::identity() },
    _inverse{ mat4::identity() },
    _normalMatrix{ mat3::identity()},
    visible{true}
  {}

  const char* name() const { return _name.c_str(); }
  void setName(const std::string& actorName){  _name = actorName; }

  bool isVisible() const { return visible; }
  void setVisible(bool v){ visible = v; }

  // Retorna a primitiva geométrica associada.
  Shape3* shape() const { return _shape; }

  // Acesso e modificação da matriz de transformação (Model Matrix).
  const mat4f transform() const { return _transform; }
  
  // Define a transformação e atualiza as matrizes derivadas (Inversa e Normal).
  void setTransform(const mat4f& transform)
  {
    _transform = transform;

    // Calcula a inversa para transformações de raio (World -> Local).
    if (!_transform.inverse(_inverse, cg::math::Limits<float>::eps()))
      _inverse = mat4::identity();

    // Calcula a matriz normal (Transposta da Inversa) para correção de normais sob escala não uniforme.
    _normalMatrix = (mat3(_inverse)).transpose();
  }

  const mat4& inverseTransform() const { return _inverse; }
  const mat3& normalMatrix() const { return _normalMatrix; }

  // Gerenciamento do material PBR.
  const PBRMaterial* pbrMaterial() const { return _material; }
  PBRMaterial* pbrMaterial() { return _material; }
  void setPBRMaterial(PBRMaterial* material) { _material = material; }

  // --- Interfaces para Estruturas de Aceleração (BVH) ---

  // Calcula a AABB (Axis-Aligned Bounding Box) no espaço do mundo.
  Bounds3f bounds() const
  {
    if (_shape == nullptr)
      return Bounds3f{};
    
    Bounds3f localBounds = _shape->bounds();
    // Aplica a transformação afim à AABB local para obter a AABB global.
    Bounds3f globalBounds{localBounds, _transform};
    
    return globalBounds;
  }

  // Teste de interseção simplificado (apenas booleano).
  bool intersect(const Ray3f& ray) const
  {
    if (_shape == nullptr)
      return false;
    
    // Transforma o raio do espaço Mundo para o espaço Local do objeto.
    const auto& invTransform = _inverse;
    Ray3f localRay;
    localRay.origin = invTransform.transform3x4(ray.origin);
    localRay.direction = invTransform.transformVector(ray.direction).versor();
    localRay.tMin = ray.tMin;
    localRay.tMax = ray.tMax;
    
    float distance = ray.tMax;
    return _shape->intersect(localRay, distance);
  }

  // Teste de interseção detalhado, preenchendo a estrutura Intersection.
  bool intersect(const Ray3f& ray, Intersection& hit) const
  {
    if (_shape == nullptr)
      return false;
    
    // Transformação do Raio: Mundo -> Local
    const auto& invTransform = _inverse;
    Ray3f localRay;
    localRay.origin = invTransform.transform3x4(ray.origin);
    localRay.direction = invTransform.transformVector(ray.direction).versor();
    localRay.tMin = ray.tMin;
    localRay.tMax = hit.distance; // Otimização: ignora interseções mais distantes que a atual.
    
    float localDistance = hit.distance;
    
    // Interseção na Primitiva (Espaço Local)
    if (_shape->intersect(localRay, localDistance))
    {
      // Conversão da Distância: Local -> Mundo
      // Necessário pois a escala do objeto afeta o valor de 't'.
      vec3f localPoint = localRay(localDistance);
      vec3f worldPoint = _transform.transform3x4(localPoint);
      vec3f toPoint = worldPoint - ray.origin;
      float worldDistance = toPoint.length();
      
      // Validação de direcionalidade e limites de profundidade.
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
  Reference<PBRMaterial> _material;
  mat4 _transform;
  mat4 _inverse;
  mat3 _normalMatrix;
};

}