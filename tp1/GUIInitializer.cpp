#include "GUIInitializer.h"
#include "MainWindow.h"
#include "Scene.h"
#include "graphics/Camera.h"
#include "PBRActor.h"

namespace cg
{

GUIInitializer::GUIInitializer(MainWindow& window) :
  _window(window),
  _selectedActorIndex(-1),
  _resetCooldown(0)
{
}

void GUIInitializer::draw()
{
  // Configuração inicial da janela principal de controles.
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

  ImGui::Begin("TP1 - PBR Controls");

  if (_resetCooldown > 0)
  {
      _resetCooldown--;
      ImGui::Text("Resetting scene...");
      ImGui::End();
      
      // Garante que o índice esteja limpo quando o cooldown acabar
      if (_resetCooldown == 0)
          _selectedActorIndex = -1;
          
      return; 
  }

  if (ImGui::CollapsingHeader("Scene Info", ImGuiTreeNodeFlags_DefaultOpen))
    drawSceneControls();

  if (_selectedActorIndex == -2)
  {
      ImGui::End();
      return; 
  }

  if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
    drawRendererControls();

  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    drawCameraControls();

  if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
    drawLightControls();

  if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
    drawMaterialControls();

  ImGui::Separator();
  // Exibição de métricas de desempenho (tempo de quadro e quadros por segundo).
  ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", _window.deltaTime() * 1000.0f, ImGui::GetIO().Framerate);

  ImGui::End();
}

void GUIInitializer::drawRendererControls()
{
  bool useRayCaster = _window.useRayCaster();
  
  // Alternância entre pipeline de Rasterização (OpenGL) e Ray Casting (CPU).
  if (ImGui::Checkbox("Use Ray Caster", &useRayCaster))
    _window.setUseRayCaster(useRayCaster);
  
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

  // Feedback visual sobre o estado da estrutura de aceleração espacial.
  if (_window.useRayCaster() && _window.rayCaster())
    ImGui::Text("BVH: Active (RayCaster)");
  else
    ImGui::Text("BVH: Inactive");
  
  // Edição da cor de fundo (Background).
  float bg[3] = { scene->backgroundColor.r, scene->backgroundColor.g, scene->backgroundColor.b };
  if (ImGui::ColorEdit3("Background", bg))
    scene->backgroundColor = Color(bg[0], bg[1], bg[2]);

  ImGui::Spacing();
  if (ImGui::Button("Reset Scene Geometry"))
  {
    _window.scheduleSceneReset();
    _resetCooldown = 2;
    _selectedActorIndex = -2; // Invalida seleção para evitar acesso a ponteiros pendentes.
    ImGui::SetWindowFocus(nullptr);
  }
}

void GUIInitializer::drawCameraControls()
{
  auto camera = _window.getCamera();
  if (!camera) return;

  bool useParallelProjection = camera->projectionType() == Camera::Parallel;

  // Manipulação de projeção.
  if (ImGui::Checkbox("Projeção Ortográfica", &useParallelProjection))
    camera->setProjectionType(useParallelProjection ? Camera::Parallel : Camera::Perspective);

  // Manipulação de Posição.
  vec3f pos = camera->position();
  if (ImGui::DragFloat3("Position", (float*)&pos, 0.1f))
    camera->setPosition(pos);

  // Manipulação de Orientação (Ângulos de Euler).
  vec3f euler = camera->eulerAngles();
  if (ImGui::DragFloat3("Rotation", (float*)&euler, 1.0f))
    camera->setEulerAngles(euler);

  // Campo de Visão (FOV).
  float fov = camera->viewAngle();
  if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f))
    camera->setViewAngle(fov);

  // Planos de Recorte (Near/Far).
  float n, f;
  camera->clippingPlanes(n, f);
  bool changed = false;
  if (ImGui::DragFloat("Near", &n, 0.01f, 0.01f, 10.0f)) changed = true;
  if (ImGui::DragFloat("Far", &f, 0.5f, 1.0f, 1000.0f)) changed = true;
  if (changed) camera->setClippingPlanes(n, f);

  // Predefinições de Câmera.
  ImGui::Separator();
  if (ImGui::Button("Front")) { camera->setPosition({0,0,15}); camera->setEulerAngles({0,0,0}); }
  ImGui::SameLine();
  if (ImGui::Button("Top")) { camera->setPosition({0,15,0}); camera->setEulerAngles({-90,0,0}); }
}

void GUIInitializer::drawLightControls()
{
  auto scene = _window.scene();
  if(!scene) return;

  const auto& lights = scene->lights();
  
  int i = 0;
  for (auto light : lights)
  {
    // Utiliza ID único para manter estado da árvore na GUI.
    ImGui::PushID(i++);
    if (ImGui::TreeNode(light->name()))
    {
      bool on = light->isTurnedOn();
      if(ImGui::Checkbox("Enabled", &on)) light->turnOn(on);

      vec3f pos = light->position();
      if (ImGui::DragFloat3("Pos", (float*)&pos, 0.1f)) light->setPosition(pos);

      float col[3] = { light->color.r, light->color.g, light->color.b };
      if (ImGui::ColorEdit3("Color", col)) light->color = Color(col[0], col[1], col[2]);

      // Seleção do tipo de decaimento (atenuação) da luz.
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
  if(!scene) return;

  const auto& actors = scene->actors();

  if (actors.empty()) return;

  // Seletor Dropdown para escolha de objetos na cena.
  const char* currentName = (_selectedActorIndex >= 0 && _selectedActorIndex < actors.size()) 
                ? actors[_selectedActorIndex]->name() 
                : "Select Actor...";

  if (ImGui::BeginCombo("Target", currentName))
  {
    for (int i = 0; i < (int)actors.size(); i++)
    {
      bool isSelected = (_selectedActorIndex == i);
      if (ImGui::Selectable(actors[i]->name(), isSelected))
      {
        _window.setSelectedActor(actors[i].get());
        PBRRenderer* renderer = _window.pbrRenderer();
        if (renderer) renderer->setSelectedActor(actors[i].get());
        _selectedActorIndex = i;
      }
      if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  PBRActor* actor = getSelectedActor();
  if (!actor) return;

  ImGui::Separator();
  
  // Edição direta das propriedades do Material PBR.
  PBRMaterial* mat = actor->pbrMaterial();

  float od[3] = { mat->Od.r, mat->Od.g, mat->Od.b };
  if (ImGui::ColorEdit3("Albedo (Od)", od)) mat->Od = Color(od[0], od[1], od[2]);

  float os[3] = { mat->Os.r, mat->Os.g, mat->Os.b };
  if (ImGui::ColorEdit3("F0 (Os)", os)) mat->Os = Color(os[0], os[1], os[2]);

  ImGui::SliderFloat("Roughness", &mat->roughness, 0.01f, 1.0f);
  ImGui::SliderFloat("Metallic", &mat->metalness, 0.0f, 1.0f);

  // Aplicação de Materiais Predefinidos.
  ImGui::Separator();
    ImGui::Text("Presets:");
  float r = mat->roughness;
  if (ImGui::Button("Gold")) actor->setPBRMaterial(PBRMaterial::gold(r));
  ImGui::SameLine();
  if (ImGui::Button("Silver")) actor->setPBRMaterial(PBRMaterial::silver(r));
  ImGui::SameLine();
  if (ImGui::Button("Copper")) actor->setPBRMaterial(PBRMaterial::copper(r));
  ImGui::SameLine();
  if (ImGui::Button("Aluminum")) actor->setPBRMaterial(PBRMaterial::aluminum(r));
  
  if (ImGui::Button("Titanium")) actor->setPBRMaterial(PBRMaterial::titanium(r));
  ImGui::SameLine();
  if (ImGui::Button("Plastic")) actor->setPBRMaterial(PBRMaterial::dielectric(Color(0.8f, 0.2f, 0.2f), r));
  
  // Controle simples de Transformação (Translação).
  ImGui::Separator();
  vec3f pos = actor->position();
  if (ImGui::DragFloat3("Actor Pos", (float*)&pos, 0.1f))
    actor->setPosition(pos);

  bool visible = actor->isVisible();
  if (ImGui::Checkbox("Visible", &visible))
    actor->setVisible(visible);
}

PBRActor* GUIInitializer::getSelectedActor()
{
  if (_selectedActorIndex == -2 || _resetCooldown > 0)
      return nullptr;

  auto scene = _window.scene();
  if (!scene) return nullptr;

  if (_selectedActorIndex >= scene->actorCount())
  {
    _selectedActorIndex = -1;
    return nullptr;
  }

  PBRActor* clickedActor = _window.selectedActor();
  
  if (clickedActor != nullptr)
  {
      const auto& actors = scene->actors();
      for (size_t i = 0; i < actors.size(); ++i)
      {
          if (actors[i].get() == clickedActor)
          {
              _selectedActorIndex = (int)i;
              return clickedActor;
          }
      }
  }

  if (_selectedActorIndex >= 0 && _selectedActorIndex < scene->actorCount())
    return scene->actors()[_selectedActorIndex].get();

  return nullptr;
}

}