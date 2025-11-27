//[]---------------------------------------------------------------[]
//|                                                                 |
//| PBRRenderer.h                                                   |
//|                                                                 |
//| Physically-Based Rendering renderer for TP1                    |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "graphics/Camera.h"
#include "Scene.h"
#include "PBRMaterial.h"
#include "geometry/TriangleMesh.h"
//#include "graphics/GLSL.h"
#include "core/Globals.h"
//#include <GL/glew.h>

namespace cg
{ // begin namespace cg

//
// PBRRenderer: renderizador com PBR
//
    class PBRRenderer
    {
    public:
        PBRRenderer(Scene& scene, Camera& camera);
        ~PBRRenderer();

        void render();

        // Acessar cena e c�mera
        Scene* scene() const { return _scene; }
        Camera* camera() const { return _camera; }

        // Configurar tamanho da imagem
        void setImageSize(int width, int height)
        {
            _viewport.w = width;
            _viewport.h = height;
        }

        // Renderizar malha com material PBR
        void drawMeshPBR(const TriangleMesh& mesh,
            const PBRMaterial& material,
            const mat4f& transform,
            const mat3f& normalMatrix);
        
        // Desenhar wireframe do objeto selecionado
        void drawSelectedActorWireframe(PBRActor* actor);
        
        // Definir ator selecionado para destacar
        void setSelectedActor(PBRActor* actor) { _selectedActor = actor; }

    protected:
        struct Viewport
        {
            int w, h;
        };

        Scene* _scene;
        Camera* _camera;
        Viewport _viewport;

        void update();
        void beginRender();
        void endRender();
        void renderLights();
        void renderActors();
        void renderMaterial(const PBRMaterial& material);

    private:
        struct PBRData;
        PBRData* _pbrData;
        PBRActor* _selectedActor = nullptr;

    }; // PBRRenderer

} // end namespace cg