#include "RayCaster.h"
#include "graphics/Light.h"
#include <cmath>
#include <limits>
#include <thread>
#include <vector>
#include <atomic>

namespace cg
{

constexpr float PI = 3.14159265359f;
constexpr float MIN_SPEC = 0.04f; // Reflectância base para dielétricos (F0)
constexpr float EPSILON = 1e-4f;  // Bias para evitar auto-interseção (Shadow Acne)

void RayCaster::buildBVH()
{
  std::vector<Reference<PBRActor>> actors;
  
  for (auto actor : _scene->actors())
  {
    if (actor->isVisible())
      actors.push_back(actor);
  }
  
  if (!actors.empty())
  {
    // Inicializa BVH.
    _bvh = new BVH<PBRActor>{std::move(actors), 8, BVHBase::SplitMethod::SAH};
    auto bounds = _bvh->bounds();
  }
}

// Gera o raio primário a partir da câmera para as coordenadas de pixel (x, y).
void RayCaster::setPixelRay(float x, float y, Ray3f& ray)
{
  auto p = imageToWindow(x, y);
  const auto& m = _camera->cameraToWorldMatrix();
  
  // Extração dos vetores base do sistema de coordenadas da câmera (VRC).
  vec3f n = vec3f{m[2].x, m[2].y, m[2].z}; // Forward (negativo)
  
  float F, B;
  _camera->clippingPlanes(F, B);
  
  if (_camera->projectionType() == Camera::Perspective)
  {
    // Projeção Perspectiva: Origem no centro de projeção.
    ray.origin = _camera->position();
    ray.direction = (p - _camera->nearPlane() * n).versor();
  }
  else
  {
    // Projeção Ortográfica: Raios paralelos.
    ray.origin = _camera->position() + p;
    ray.direction = -n;
  }
  
  ray.tMin = F;
  ray.tMax = B;
}

// Mapeia coordenadas do espaço de imagem (raster) para o plano de visualização no espaço do mundo.
vec3f RayCaster::imageToWindow(float x, float y) const
{
  const auto& m = _camera->cameraToWorldMatrix();
  vec3f u = vec3f{m[0].x, m[0].y, m[0].z};
  vec3f v = vec3f{m[1].x, m[1].y, m[1].z};
  
  float wh = _camera->windowHeight();
  float Iw = 1.0f / _viewport.w;
  float Ih = 1.0f / _viewport.h;
  
  // Ajuste de aspect ratio para manter proporções corretas (Fit).
  float Vw, Vh;
  if (_viewport.w >= _viewport.h)
    Vw = (Vh = wh) * _viewport.w * Ih;
  else
    Vh = (Vw = wh) * _viewport.h * Iw;
  
  return Vw * (x * Iw - 0.5f) * u + Vh * (y * Ih - 0.5f) * v;
}

// Realiza o teste de interseção do raio com a cena.
bool RayCaster::intersect(const Ray3f& ray, Intersection& hit)
{
  hit.object = nullptr;
  hit.distance = ray.tMax;
  
  if (!_bvh || _bvh->empty())
    return false;
  
  // Tentativa de interseção via BVH.
  bool found = _bvh->intersect(ray, hit);
  
  if (!found && _bruteIntersect)
  {
    // Fallback: Busca linear para garantir robustez caso o BVH falhe ou não cubra objetos dinâmicos.
    PBRActor* closestActor = nullptr;
    float closestDistance = ray.tMax;
    
    //printf("intersect 1");

    for (const auto& actorRef : _scene->actors())
    {
      // Extrai o ponteiro bruto (T*) do wrapper Reference<T>
      PBRActor* actor = actorRef.get();

      if (!actor->isVisible())
        continue;
      
      Intersection tempHit;
      tempHit.distance = closestDistance;
      if (actor->intersect(ray, tempHit))
      {
        if (tempHit.distance < closestDistance)
        {
          closestDistance = tempHit.distance;
          closestActor = actor;
        }
      }
    }
    
    //printf("intersect 2\n");
    
    if (closestActor != nullptr)
    {
      hit.object = closestActor;
      hit.distance = closestDistance;
      return true;
    }
  }
  
  return found;
}

// Implementação do modelo de iluminação PBR.
Color RayCaster::calculatePBR(const vec3f& P, const vec3f& N, const PBRMaterial* material)
{
  vec3f V = (_camera->position() - P).versor(); // Vetor View
  vec3f normal = N.versor();
  
  // Cálculo da refletância base (F0).
  vec3f F0 = vec3f{material->Os.r, material->Os.g, material->Os.b} * material->metalness + vec3f{MIN_SPEC, MIN_SPEC, MIN_SPEC} * (1.0f - material->metalness);
  
  // Albedo corrigido pela metalicidade.
  vec3f albedo = vec3f{material->Od.r, material->Od.g, material->Od.b} * (1.0f - material->metalness);
  
  Color Lo{0, 0, 0};
  
  // Integração da contribuição das luzes analíticas.
  for (const auto& light : _scene->lights())
  {
    if (!light->isTurnedOn())
      continue;
    
    vec3f L; // Vetor Light
    float d;
    
    if (!light->lightVector(P, L, d))
      continue;
    
    float NdotL = normal.dot(L);
    if (NdotL <= 0)
      continue;
    
    // Cálculo de sombras (Shadow Ray).
    Ray3f shadowRay{P + L * EPSILON, L};
    shadowRay.tMax = d;
    
    Intersection shadowHit;
    if (intersect(shadowRay, shadowHit))
      continue; // Ponto ocluído.
    
    Color radiance = light->lightColor(d);
    
    // Vetor Halfway (H) para o modelo de microfacetas.
    vec3f H = (V + L).versor();
    float NdotV = std::max(normal.dot(V), 0.0f);
    float NdotH = std::max(normal.dot(H), 0.0f);
    float VdotH = std::max(V.dot(H), 0.0f);
    
    // Parâmetro de rugosidade alfa.
    float a = material->roughness * material->roughness;
    float a2 = a * a;
    
    // Distribuição Normal (NDF) - Trowbridge-Reitz GGX.
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    float D = a2 / (PI * denom * denom);
    
    // Função de Geometria (G) - Smith com aproximação Schlick-GGX.
    float k = (material->roughness + 1.0f) * (material->roughness + 1.0f) / 8.0f;
    float G1V = NdotV / (NdotV * (1.0f - k) + k);
    float G1L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G1V * G1L;
    
    // Fresnel (F) - Aproximação de Schlick.
    vec3f F = F0 + (vec3f{1, 1, 1} - F0) * std::pow(1.0f - VdotH, 5.0f);
    
    // BRDF Especular (Cook-Torrance).
    float denomSpec = 4.0f * NdotV * NdotL + 1e-6f; 
    vec3f specular = F * (D * G / denomSpec);
    
    // Conservação de energia: kD + kS = 1.
    vec3f kS = F;
    vec3f kD = (vec3f{1, 1, 1} - kS) * (1.0f - material->metalness);
    
    // Equação de renderização final para esta luz.
    vec3f radianceVec = vec3f{radiance.r, radiance.g, radiance.b};
    vec3f diffuseTerm = kD * albedo * (1.0f / PI); // Lambertian Diffuse.
    vec3f contribution = (diffuseTerm + specular) * radianceVec * NdotL;
    
    Lo.r += contribution.x;
    Lo.g += contribution.y;
    Lo.b += contribution.z;
  }
  
  // Termo de luz ambiente simples (substituto para IBL/Global Illumination).
  vec3f ambient = vec3f{0.03f, 0.03f, 0.03f} * albedo;
  Lo.r += ambient.x;
  Lo.g += ambient.y;
  Lo.b += ambient.z;
  
  return Lo;
}

// Determina a cor de um ponto dado uma interseção (Cálculo de Shading).
Color RayCaster::shade(const Ray3f& ray, const Intersection& hit)
{
  auto actor = (PBRActor*)hit.object;
  if (actor == nullptr)
    return background();
  
  vec3f P = ray(hit.distance);
  
  auto shape = actor->shape();
  if (shape == nullptr)
    return background();
  
  // Transformação Espaço Mundo -> Espaço Objeto.
  const auto& invTransform = actor->inverseTransform();
  vec3f localP = invTransform.transform3x4(P);
  
  vec3f localN = shape->normalAt(localP);
  
  // Transformação da normal usando a Matriz Normal (Transposta da Inversa).
  const auto& normalMatrix = actor->normalMatrix();
  vec3f N = vec3f{
    normalMatrix[0].dot(localN),
    normalMatrix[1].dot(localN),
    normalMatrix[2].dot(localN)
  }.versor();
  
  const auto * material = actor->pbrMaterial();
  
  return calculatePBR(P, N, material);
}

Color RayCaster::background() const
{
  return _scene->backgroundColor;
}

// Loop principal de renderização paralelizado.
void RayCaster::renderImage(Camera* camera, Image* image)
{
    if (!camera || !image) return;
    
    _camera = camera;
    const int W = _viewport.w;
    const int H = _viewport.h;
    if (W <= 0 || H <= 0) return;
    
    // Câmera
    const vec3f camPos   = _camera->position();
    const mat4f& camMat  = _camera->cameraToWorldMatrix();
    const vec3f camRight = vec3f{camMat[0].x, camMat[0].y, camMat[0].z};
    const vec3f camUp    = vec3f{camMat[1].x, camMat[1].y, camMat[1].z};
    const vec3f camFwd   = vec3f{camMat[2].x, camMat[2].y, camMat[2].z};

    // Cálculos de Viewport e Aspect Ratio
    float winHeight = _camera->windowHeight();
    float aspect = (float)W / (float)H;
    float winWidth = winHeight * aspect; 

    float pixelWidthWorld, pixelHeightWorld;
    if (W >= H) {
        pixelHeightWorld = winHeight / H;
        pixelWidthWorld  = (winHeight * aspect) / W;
    } else {
        pixelWidthWorld  = winWidth / W;
        pixelHeightWorld = (winWidth / aspect) / H;
    }

    const vec3f deltaU = camRight * pixelWidthWorld;
    const vec3f deltaV = camUp * pixelHeightWorld;
    const vec3f viewDir = -camFwd; 
    
    // Define centro do plano de visão
    vec3f viewPlaneCenter;
    if (_camera->projectionType() == Camera::Perspective)
        viewPlaneCenter = camPos + (viewDir * _camera->nearPlane());
    else
        viewPlaneCenter = camPos + (viewDir * 1.0f);

    const vec3f topLeftPixelCenter = viewPlaneCenter 
                                   - (camRight * (pixelWidthWorld * W * 0.5f)) 
                                   - (camUp * (pixelHeightWorld * H * 0.5f))
                                   + (deltaU * 0.5f) + (deltaV * 0.5f);

    float clipF, clipB;
    _camera->clippingPlanes(clipF, clipB);
    
    const bool isOrthoProjection = (_camera->projectionType() == Camera::Parallel);

    // Setup de Concorrência ---
    const unsigned int hw = std::thread::hardware_concurrency();
    const int numThreads = std::max(1u, hw ? hw : 1u);
    const int linesPerThread = std::max(1, H / numThreads);
    
    ImageBuffer framebuffer(W, H);
    std::atomic<bool> cancelFlag{ false };

    // Kernel de Renderização
    auto renderLoop = [&](auto IsOrthoTag, int y0, int y1) 
    {
        constexpr bool IsOrtho = decltype(IsOrthoTag)::value;

        Ray3f ray;
        ray.tMin = clipF;
        ray.tMax = clipB;
        Intersection hit;

        if constexpr (!IsOrtho) ray.origin = camPos; // Se for Perspectiva, a origem é constante.
        if constexpr (IsOrtho)  ray.direction = viewDir; // Se for Orto, a direção é constante.

        for (int y = y0; y < y1; ++y)
        {
            if (cancelFlag.load(std::memory_order_relaxed)) return;

            const vec3f rowStart = topLeftPixelCenter + (deltaV * (float)y);

            for (int x = 0; x < W; ++x)
            {
                // Cálculo do pixel no mundo
                const vec3f pixelWorldPos = rowStart + (deltaU * (float)x);

                if constexpr (!IsOrtho) // Perspectiva: Direção varia, Origem fixa
                  ray.direction = (pixelWorldPos - camPos).versor();
                else // Ortográfica: Origem varia, Direção fixa
                    ray.origin = pixelWorldPos;

                hit.object = nullptr;
                hit.distance = ray.tMax;
                
                Color finalColor = background();
                if (intersect(ray, hit))
                    finalColor = shade(ray, hit);
                
                framebuffer(x, y).set(clampColor(finalColor));
            }
        }
    };

    // Dispatch
    auto worker = [&](int y0, int y1) {
        if (isOrthoProjection)
            renderLoop(std::true_type{}, y0, y1);  // Instancia versão Orto
        else
            renderLoop(std::false_type{}, y0, y1); // Instancia versão Perspectiva
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        int y0 = i * linesPerThread;
        int y1 = (i == numThreads - 1) ? H : y0 + linesPerThread;
        threads.emplace_back(worker, y0, y1);
    }

    for (auto& t : threads)
        if (t.joinable()) t.join();

    if (!cancelFlag.load()) image->setData(framebuffer);
}

// Executa seleção de objetos via Ray Casting (Picking).
PBRActor* RayCaster::selectActor(int x, int y)
{
  if (_bvh == nullptr || _bvh->empty())
  {
    buildBVH();
    if (_bvh == nullptr || _bvh->empty())
      return nullptr;
  }
  
  Ray3f ray;
  setPixelRay((float)x + 0.5f, (float)y + 0.5f, ray);
  
  Intersection hit;
  hit.distance = ray.tMax;
  
  if (intersect(ray, hit))
    return (PBRActor*)hit.object;
  
  return nullptr;
}

}