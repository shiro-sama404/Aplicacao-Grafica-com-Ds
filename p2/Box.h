#pragma once
#include "Shape3.h"
#include <geometry/Bounds3.h> 

class Box final : public Shape3
{
public:
    Box() : Shape3{ GLGraphics3::box() } {
        //O construtor inicializa a malhacom a caixa padrão da biblioteca (cubo 2x2)
    }

    bool intersect(const Ray3<float>& ray, float& t) const override;
    vec3 normalAt(const vec3& p) const override;

    Bounds3f bounds() const override; 
};