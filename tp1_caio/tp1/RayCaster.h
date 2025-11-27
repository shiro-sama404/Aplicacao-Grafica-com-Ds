#pragma once

#include "Scene.h"
#include "PBRActor.h"
#include "graphics/Camera.h"
#include "graphics/GLImage.h"
#include "geometry/Ray.h"
#include "geometry/Intersection.h"
#include "geometry/BVH.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace cg
{
class RayCaster
{
public:
  RayCaster(Scene& scene, Camera& camera):
    _scene{&scene},
    _camera{&camera},
    _bvh{nullptr}
  {
    buildBVH();
  }

  ~RayCaster()
  {
  }

  // Define as dimensões do viewport de renderização.
  void setImageSize(int width, int height)
  {
    _viewport.w = width;
    _viewport.h = height;
  }

  // Executa o pipeline de renderização (geração de raios, traçado e shading) para preencher a imagem.
  void renderImage(Image& image);

  // Executa o algoritmo de picking (seleção) disparando um raio através das coordenadas de tela (x, y).
  PBRActor* selectActor(int x, int y);
  
  Camera* camera() const { return _camera; }
  
  // Reconstrói a estrutura de aceleração espacial (necessário se a geometria da cena for alterada).
  void rebuildBVH() { buildBVH(); }

private:
  struct Viewport
  {
    int w, h;
  };

  Scene* _scene;
  Camera* _camera;
  Viewport _viewport;
  Reference<BVH<PBRActor>> _bvh;

  // Métodos internos do pipeline de Ray Tracing
  
  void buildBVH();
  
  // Gera o raio primário a partir da câmera.
  void setPixelRay(float x, float y, Ray3f& ray);
  
  // Dispara o processo de renderização para um único pixel.
  Color shoot(float x, float y);
  
  // Função recursiva para acompanhamento do caminho do raio.
  Color trace(const Ray3f& ray, int depth = 0);
  
  // Teste de interseção contra a estrutura de aceleração.
  bool intersect(const Ray3f& ray, Intersection& hit);
  
  // Calcula a cor final de um ponto de interseção.
  Color shade(const Ray3f& ray, const Intersection& hit);
  
  // Aplica o modelo de iluminação Cook-Torrance BRDF.
  Color calculatePBR(const vec3f& P, const vec3f& N, const PBRMaterial* material);
  
  // Mapeia coordenadas de raster para coordenadas de janela (View Plane).
  vec3f imageToWindow(float x, float y) const;
  
  Color background() const;

};

}