// GUIInitializer.cpp
#include "GUIInitializer.h"
#include "MainWindow.h"
#include "Scene.h"
#include "graphics/Camera.h"   // Necessário para manipular a câmera
#include "PBRActor.h" // Necessário para PBRMaterial

namespace cg {

    GUIInitializer::GUIInitializer(MainWindow& window) :
        _window(window)
    {
    }

    void GUIInitializer::draw()
    {
        // Configuração da Janela ImGui
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

        ImGui::Begin("TP1 - PBR Controls");

        if (ImGui::CollapsingHeader("Scene Info", ImGuiTreeNodeFlags_DefaultOpen))
            drawSceneControls();

        if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
            drawRendererControls();

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            drawCameraControls();

        if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
            drawLightControls();

        if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
            drawMaterialControls();

        ImGui::Separator();
        ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", _window.deltaTime() * 1000.0f, ImGui::GetIO().Framerate);
        
        ImGui::End();
        
        // Janela de inspeção do ator selecionado
        drawActorInspector();
    }

    void GUIInitializer::drawRendererControls()
    {
        bool useRayCaster = _window.useRayCaster();
        
        if (ImGui::Checkbox("Use Ray Caster", &useRayCaster))
        {
            _window.setUseRayCaster(useRayCaster);
        }
        
        ImGui::Text("Active Renderer: %s", useRayCaster ? "RayCaster (with BVH)" : "PBRRenderer (OpenGL)");
        
        if (useRayCaster)
        {
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "RayCaster rendering active");
            ImGui::Text("BVH acceleration enabled");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "OpenGL rendering active");
            ImGui::Text("Click on objects to select them");
        }
    }

    void GUIInitializer::drawSceneControls()
    {
        auto scene = _window.scene();
        if(!scene) return;

        ImGui::Text("Scene: %s", scene->name());
        ImGui::Text("Actors: %d | Lights: %d", scene->actorCount(), scene->lightCount());

        // Mostrar informação sobre BVH se RayCaster estiver ativo
        if (_window.useRayCaster() && _window.rayCaster())
        {
            ImGui::Text("BVH: Active (RayCaster)");
        }
        
        // Background Color
        float bg[3] = { scene->backgroundColor.r, scene->backgroundColor.g, scene->backgroundColor.b };
        if (ImGui::ColorEdit3("Background", bg))
        {
            scene->backgroundColor = Color(bg[0], bg[1], bg[2]);
        }

        // Reset Button
        ImGui::Spacing();
        if (ImGui::Button("Reset Scene Geometry"))
        {
            _window.resetScene();
            _selectedActorIndex = -1; // Reseta seleção para evitar crash
        }
    }

    void GUIInitializer::drawCameraControls()
    {
        auto camera = _window.camera();
        if (!camera) return;

        // Position
        vec3f pos = camera->position();
        if (ImGui::DragFloat3("Position", (float*)&pos, 0.1f))
            camera->setPosition(pos);

        // Rotation
        vec3f euler = camera->eulerAngles();
        if (ImGui::DragFloat3("Rotation", (float*)&euler, 1.0f))
            camera->setEulerAngles(euler);

        // FOV
        float fov = camera->viewAngle();
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f))
            camera->setViewAngle(fov);

        // Planes
        float n, f;
        camera->clippingPlanes(n, f);
        bool changed = false;
        if (ImGui::DragFloat("Near", &n, 0.01f, 0.01f, 10.0f)) changed = true;
        if (ImGui::DragFloat("Far", &f, 0.5f, 1.0f, 1000.0f)) changed = true;
        if (changed) camera->setClippingPlanes(n, f);

        // Presets
        ImGui::Separator();
        if (ImGui::Button("Front")) { camera->setPosition({0,0,15}); camera->setEulerAngles({0,0,0}); }
        ImGui::SameLine();
        if (ImGui::Button("Top")) { camera->setPosition({0,15,0}); camera->setEulerAngles({-90,0,0}); }
    }

    void GUIInitializer::drawLightControls()
    {
        auto scene = _window.scene();
        const auto& lights = scene->lights();
        
        int i = 0;
        for (auto light : lights)
        {
            ImGui::PushID(i++);
            if (ImGui::TreeNode(light->name()))
            {
                bool on = light->isTurnedOn();
                if(ImGui::Checkbox("Enabled", &on)) light->turnOn(on);

                vec3f pos = light->position();
                if (ImGui::DragFloat3("Pos", (float*)&pos, 0.1f)) light->setPosition(pos);

                float col[3] = { light->color.r, light->color.g, light->color.b };
                if (ImGui::ColorEdit3("Color", col)) light->color = Color(col[0], col[1], col[2]);

                // Falloff
                int falloff = (int)light->falloff;
                if(ImGui::Combo("Falloff", &falloff, "None\0Linear\0Quadratic\0"))
                    light->falloff = (Light::Falloff)falloff;

                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    void GUIInitializer::drawMaterialControls()
    {
        auto scene = _window.scene();
        const auto& actors = scene->actors();

        if (actors.empty()) return;

        // Combo Box para Seleção
        const char* currentName = (_selectedActorIndex >= 0 && _selectedActorIndex < actors.size()) 
                                  ? actors[_selectedActorIndex]->name() 
                                  : "Select Actor...";

        if (ImGui::BeginCombo("Target", currentName))
        {
            for (int i = 0; i < (int)actors.size(); i++)
            {
                bool isSelected = (_selectedActorIndex == i);
                if (ImGui::Selectable(actors[i]->name(), isSelected))
                    _selectedActorIndex = i;
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        PBRActor* actor = getSelectedActor();
        if (!actor) return;

        ImGui::Separator();
        
        // USANDO A MELHORIA DE REFERÊNCIA DIRETA
        PBRMaterial* mat = actor->pbrMaterial();

        float od[3] = { mat->Od.r, mat->Od.g, mat->Od.b };
        if (ImGui::ColorEdit3("Albedo (Od)", od)) mat->Od = Color(od[0], od[1], od[2]);

        float os[3] = { mat->Os.r, mat->Os.g, mat->Os.b };
        if (ImGui::ColorEdit3("F0 (Os)", os)) mat->Os = Color(os[0], os[1], os[2]);

        ImGui::SliderFloat("Roughness", &mat->roughness, 0.01f, 1.0f);
        ImGui::SliderFloat("Metallic", &mat->metalness, 0.0f, 1.0f);

        // Presets
        ImGui::Separator();
        float r = mat->roughness;
        if (ImGui::Button("Gold")) actor->setPBRMaterial(PBRMaterial::gold(r));
        ImGui::SameLine();
        if (ImGui::Button("Plastic")) actor->setPBRMaterial(PBRMaterial::dielectric(Color(1,0,0), r));

        // Transform Controls (Extraído da matriz)
        ImGui::Separator();
        mat4f transform = actor->transform();
        vec3f pos(transform[0][3], transform[1][3], transform[2][3]);
        if (ImGui::DragFloat3("Actor Pos", (float*)&pos, 0.1f))
        {
            transform[0][3] = pos.x;
            transform[1][3] = pos.y;
            transform[2][3] = pos.z;
            actor->setTransform(transform);
        }
    }

    PBRActor* GUIInitializer::getSelectedActor()
    {
        // Primeiro, verificar se há ator selecionado via clique do mouse
        PBRActor* clickedActor = _window.selectedActor();
        if (clickedActor != nullptr)
        {
            // Atualizar índice selecionado
            auto scene = _window.scene();
            if (scene)
            {
                const auto& actors = scene->actors();
                for (size_t i = 0; i < actors.size(); ++i)
                {
                    if (actors[i] == clickedActor)
                    {
                        _selectedActorIndex = (int)i;
                        break;
                    }
                }
            }
            return clickedActor;
        }
        
        // Caso contrário, usar seleção via combo box
        auto scene = _window.scene();
        if (!scene || _selectedActorIndex < 0 || _selectedActorIndex >= scene->actorCount())
            return nullptr;
        
        return dynamic_cast<PBRActor*>(scene->actors()[_selectedActorIndex]);
    }

    void GUIInitializer::drawActorInspector()
    {
        PBRActor* actor = getSelectedActor();
        if (actor == nullptr)
            return;
        
        // Janela de inspeção flutuante
        ImGui::SetNextWindowPos(ImVec2(420, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Actor Inspector", nullptr, ImGuiWindowFlags_None);
        
        ImGui::Text("Actor: %s", actor->name());
        ImGui::Separator();
        
        // Propriedades de material
        if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen))
        {
            PBRMaterial* mat = actor->pbrMaterial();
            
            float od[3] = { mat->Od.r, mat->Od.g, mat->Od.b };
            if (ImGui::ColorEdit3("Albedo (Od)", od))
                mat->Od = Color(od[0], od[1], od[2]);
            
            float os[3] = { mat->Os.r, mat->Os.g, mat->Os.b };
            if (ImGui::ColorEdit3("F0 (Os)", os))
                mat->Os = Color(os[0], os[1], os[2]);
            
            ImGui::SliderFloat("Roughness", &mat->roughness, 0.01f, 1.0f);
            ImGui::SliderFloat("Metalness", &mat->metalness, 0.0f, 1.0f);
            
            // Presets
            ImGui::Separator();
            ImGui::Text("Presets:");
            float r = mat->roughness;
            if (ImGui::Button("Gold")) actor->setPBRMaterial(PBRMaterial::gold(r));
            ImGui::SameLine();
            if (ImGui::Button("Silver")) actor->setPBRMaterial(PBRMaterial::silver(r));
            ImGui::SameLine();
            if (ImGui::Button("Copper")) actor->setPBRMaterial(PBRMaterial::copper(r));
            
            if (ImGui::Button("Aluminum")) actor->setPBRMaterial(PBRMaterial::aluminum(r));
            ImGui::SameLine();
            if (ImGui::Button("Titanium")) actor->setPBRMaterial(PBRMaterial::titanium(r));
            ImGui::SameLine();
            if (ImGui::Button("Plastic")) actor->setPBRMaterial(PBRMaterial::dielectric(Color(0.8f, 0.2f, 0.2f), r));
        }
        
        // Transformação
        if (ImGui::CollapsingHeader("Transform"))
        {
            mat4f transform = actor->transform();
            vec3f pos(transform[0][3], transform[1][3], transform[2][3]);
            if (ImGui::DragFloat3("Position", (float*)&pos, 0.1f))
            {
                transform[0][3] = pos.x;
                transform[1][3] = pos.y;
                transform[2][3] = pos.z;
                actor->setTransform(transform);
            }
        }
        
        // Visibilidade
        bool visible = actor->isVisible();
        if (ImGui::Checkbox("Visible", &visible))
            actor->setVisible(visible);
        
        ImGui::End();
    }

} // namespace cg