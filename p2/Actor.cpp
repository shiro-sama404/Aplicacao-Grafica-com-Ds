#include "Actor.h"

//Implementação da interseção booleana (para sombras)
bool Actor::intersect(const Ray3f& ray) const
{
    if (!_shape)
        return false;

    //Transforma o raio para o espaço local
    const mat4& M_inv = inverseTransform();
    vec3 O = M_inv.transform(ray.origin);
    vec3 D = M_inv.transformVector(ray.direction);
    Ray3<float> localRay{ O, D };

    float tLocal;
    if (_shape->intersect(localRay, tLocal))
    {
        //Se encontrar, só retorna true se estiver na frente do raio
        return (tLocal > 1e-4f);
    }
    return false;
}

//Implementação da interseção com o struct da biblioteca
bool Actor::intersect(const Ray3f& ray, cg::Intersection& hit) const
{
    if (!_shape)
        return false;

    //Transforma o raio para o espaço local
    const mat4& M_inv = inverseTransform();
    vec3 O = M_inv.transform(ray.origin);
    vec3 D = M_inv.transformVector(ray.direction);
    Ray3<float> localRay{ O, D };

    float tLocal;
    if (_shape->intersect(localRay, tLocal))
    {    
        if (tLocal > 1e-4f) //Apenas verifica se está na frente do raio
        {
            //Preenche o struct cg::Intersection
            hit.distance = tLocal;
            hit.object = this; //Armazena um ponteiro para ESTE Ator
            return true;
        }
    }
    return false;
}