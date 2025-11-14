#include "Box.h"

bool Box::intersect(const Ray3<float>& ray, float& t) const
{
    //Define a caixa local (cubo 2x2 centrado na origem)
    Bounds3f localBox{ vec3f{ -1.0f }, vec3f{ 1.0f } };
    float t0, t1;

    //Usa o método de interseção da biblioteca
    if (!localBox.intersect(ray, t0, t1))
        return false;

    constexpr float EPS = 1e-4f;

    //Verifica a interseção mais próxima (t0)
    if (t0 > EPS)
    {
        t = t0;
        return true;
    }

    //Se t0 estiver atrás, verifica a interseção de saída (t1)
    if (t1 > EPS)
    {
        t = t1;
        return true;
    }

    //Ambas interseções estão atrás do raio
    return false;
}

vec3 Box::normalAt(const vec3& p) const
{
    //Encontra o componente (x, y, ou z) com o maior valor absoluto.
    //A normal de um cubo alinhado aos eixos é sempre um dos eixos.
    float absX = std::abs(p.x);
    float absY = std::abs(p.y);
    float absZ = std::abs(p.z);

    if (absX > absY && absX > absZ)
        return vec3{ p.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
    
    if (absY > absX && absY > absZ)
        return vec3{ 0.0f, p.y > 0.0f ? 1.0f : -1.0f, 0.0f };

    return vec3{ 0.0f, 0.0f, p.z > 0.0f ? 1.0f : -1.0f };
}

Bounds3f Box::bounds() const
{
    //Retorna os limites da caixa local
    return Bounds3f{ vec3f{ -1.0f }, vec3f{ 1.0f } };
}