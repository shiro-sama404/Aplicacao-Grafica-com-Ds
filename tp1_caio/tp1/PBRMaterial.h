//[]---------------------------------------------------------------[]
//|                                                                 |
//| PBRMaterial.h                                                   |
//|                                                                 |
//| PBR Material structure for TP1                                 |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "graphics/Color.h"

namespace cg
{ // begin namespace cg

//
// PBRMaterial: estrutura para materiais PBR
//
class PBRMaterial
{
public:
    Color Od;        // Diffuse reflection color
    Color Os;        // Specular reflection color
    float roughness; // Rugosidade [0,1]
    float metalness; // Metalicidade [0,1]

    // Construtor padrão: dielétrico cinza
    PBRMaterial() :
        Od(Color{ 0.5f, 0.5f, 0.5f }),
        Os(Color{ 0.04f, 0.04f, 0.04f }),
        roughness{ 0.5f },
        metalness{ 0.0f }
    {
        // do nothing
    }

    // Construtor completo
    PBRMaterial
    (
        const Color& diffuse,
        const Color& specular,
        float r,
        float m
    ) :
        Od{ diffuse },
        Os{ specular },
        roughness{ r },
        metalness{ m }
    {
        // do nothing
    }

    auto clone() const
    {
        return new PBRMaterial{*this};
    }

    // Presets para metais comuns
    static const PBRMaterial copper(float r = 0.3f)
    {
        return PBRMaterial(Color{ 0, 0, 0 }, Color{ 0.95f, 0.64f, 0.54f }, r, 1.0f);
    }

    static const PBRMaterial aluminum(float r = 0.3f)
    {
        return PBRMaterial(Color{ 0, 0, 0 }, Color{ 0.91f, 0.92f, 0.92f }, r, 1.0f);
    }

    static const PBRMaterial silver(float r = 0.3f)
    {
        return PBRMaterial(Color{ 0, 0, 0 }, Color{ 0.95f, 0.93f, 0.88f }, r, 1.0f);
    }

    static const PBRMaterial titanium(float r = 0.3f)
    {
        return PBRMaterial(Color{ 0, 0, 0 }, Color{ 0.542f, 0.497f, 0.449f }, r, 1.0f);
    }

    static const PBRMaterial gold(float r = 0.3f)
    {
        return PBRMaterial(Color{ 0, 0, 0 }, Color{ 1.0f, 0.71f, 0.29f }, r, 1.0f);
    }

    // Preset para dielétricos
    static const PBRMaterial dielectric(const Color& diffuse, float r = 0.5f)
    {
        return PBRMaterial(diffuse, Color{ 0.04f, 0.04f, 0.04f }, r, 0.0f);
    }

}; // PBRMaterial

} // end namespace cg