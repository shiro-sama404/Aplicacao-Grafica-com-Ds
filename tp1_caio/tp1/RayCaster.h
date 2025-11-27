//[]---------------------------------------------------------------[]
//|                                                                 |
//| RayCaster.h                                                     |
//|                                                                 |
//| Ray casting renderer with PBR lighting for TP1                 |
//|                                                                 |
//[]---------------------------------------------------------------[]

#pragma once

#include "Scene.h"
#include "PBRActor.h"
#include "graphics/Camera.h"
#include "graphics/Image.h"
#include "geometry/Ray.h"
#include "geometry/Intersection.h"
#include "geometry/BVH.h"
#include <vector>

namespace cg
{ // begin namespace cg

//
// RayCaster: renderizador com ray casting e iluminação PBR
//
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
    // BVH será deletado automaticamente pela Reference
  }

  void setImageSize(int width, int height)
  {
    _viewport.w = width;
    _viewport.h = height;
  }

  void renderImage(Image& image);

  // Selecionar ator através de raio disparado da câmera
  PBRActor* selectActor(int x, int y);
  
  // Acessar câmera
  Camera* camera() const { return _camera; }
  
  // Reconstruir BVH (útil quando a cena muda)
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

  void buildBVH();
  void setPixelRay(float x, float y, Ray3f& ray);
  Color shoot(float x, float y);
  Color trace(const Ray3f& ray, int depth = 0);
  bool intersect(const Ray3f& ray, Intersection& hit);
  Color shade(const Ray3f& ray, const Intersection& hit);
  Color calculatePBR(const vec3f& P, const vec3f& N, const PBRMaterial& material);
  vec3f imageToWindow(float x, float y) const;
  Color background() const;

}; // RayCaster

} // end namespace cg

