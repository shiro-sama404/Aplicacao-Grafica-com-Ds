#include <graphics/GLImage.h>
#include "Scene.h"
#include <vector> 

using namespace cg;

//Início BVH

void Scene::buildBVH()
{
    //1. Cria um vetor de Referências de Atores 
    std::vector<Reference<Actor>> actorRefs;

    //2. Preenche o vetor
    for (auto actor : _actors)
    {
        actorRefs.push_back(Reference<Actor>(actor));
    }

    //3. Constrói a BVH
    if (!actorRefs.empty())
    {
        _bvh = new BVH<Actor>(std::move(actorRefs));
    }
}

bool Scene::intersect(const Ray3f& ray, ::Intersection& hit) const
{
    if (!_bvh) 
        return false;

    //1. Cria um struct 'Intersection' (da biblioteca cg)
    cg::Intersection libHit;
    //2. Inicializa a distância 
    libHit.distance = std::numeric_limits<float>::max();

    //3. Chama a BVH. 'libHit.object' será preenchido com o Actor*
    if (!_bvh->intersect(ray, libHit))
        return false;

    //4. Traduz de 'cg::Intersection' (biblioteca) para '::Intersection' (nosso)
    hit.actor = static_cast<const Actor*>(libHit.object);
    hit.distance = libHit.distance;

    //5. Calcula o ponto de hit e a normal 
    hit.point = ray.origin + ray.direction * hit.distance;
    
    vec3 O_local = hit.actor->inverseTransform().transform(ray.origin);
    vec3 D_local = hit.actor->inverseTransform().transformVector(ray.direction);
    vec3 p_local = O_local + D_local * hit.distance;

    vec3 nLocal = hit.actor->shape()->normalAt(p_local);
    hit.normal = (hit.actor->normalMatrix() * nLocal).versor();

    return true;
}


static inline vec3 reflect(const vec3& I, const vec3& N)
{
    return I - 2.0f * I.dot(N) * N;
}

Color Scene::shade(const ::Intersection& hit, const Camera& camera) const
{
    if (!hit.actor)
        return background;

    const Actor* actor = hit.actor;
    const Material* material = actor->material();
    if (!material)
        return background;

    vec3 P = hit.point;
    vec3 N = hit.normal.versor();
    vec3 V = (camera.position() - P).versor();

    //componente ambiente
    Color result = ambientLight * material->ambient;

    //percorre todas as luzes
    for (auto light : _lights)
    {
        if (!light->isTurnedOn())
            continue;

        vec3 L;
        float distance;

        if (!light->lightVector(P, L, distance))
            continue; 

        Ray3<float> shadowRay(P + N * 1e-3f, L);
        
        //Checagem de Sombra com BVH
        if (_bvh)
        {
            cg::Intersection shadowHit;
            shadowHit.distance = distance; //Distância máxima (até a luz)

            if (_bvh->intersect(shadowRay, shadowHit))
            {
                // Se a interseção (shadowHit.distance) está antes da luz, então está na sombra
                if (shadowHit.distance < distance)
                    continue; 
            }
        }

        L = L.versor();
        Color I = light->lightColor(distance);

        //difusa
        float diff = std::max(0.0f, N.dot(L));

        result += material->diffuse * I * diff;

        //especular (do Phong)
        if (diff > 0.0f)
        {
            vec3 R = reflect(-L, N); //reflete luz incidente
            float spec = std::pow(std::max(0.0f, R.dot(V)), material->shine);
            result += material->specular * I * spec;
        }
    }

    return result;
}

static inline Color clampColor(const Color& c)
{
    auto clamp01 = [](float v) { return std::min(1.0f, std::max(0.0f, v)); };
    return Color{ clamp01(c.r), clamp01(c.g), clamp01(c.b), clamp01(c.a) };
}

void Scene::render(const Camera& camera, Image& image) const
{
    const int W = image.width();
    const int H = image.height();
    if (W <= 0 || H <= 0)
        return;

    const unsigned int hw = std::thread::hardware_concurrency();
    const int numThreads = std::max(1u, hw ? hw : 1u);
    const int linesPerThread = std::max(1, H / numThreads);

    std::atomic<bool> cancelFlag{ false };
    const uint32_t camStamp = camera.timestamp();

    ImageBuffer framebuffer(W, H);

    const vec3 camPos = camera.position();
    const float nearP = camera.nearPlane();
    const float viewH = camera.windowHeight();
    const float aspect = camera.aspectRatio();
    const float viewW = viewH * aspect;
    const float invW = 1.0f / float(W);
    const float invH = 1.0f / float(H);

    auto renderBlock = [&](int y0, int y1)
        {
            for (int y = y0; y < y1 && !cancelFlag.load(); ++y)
            {
                float ndcY = (0.5f - (y + 0.5f) * invH) * viewH;
                for (int x = 0; x < W; ++x)
                {
                    float ndcX = ((x + 0.5f) * invW - 0.5f) * viewW;

                    vec3 pCamera{ ndcX, ndcY, -nearP };
                    vec3 pWorld = camera.cameraToWorld(pCamera);
                    vec3 dir = (pWorld - camPos).versor();

                    Ray3<float> ray(camPos, dir);
                    
                    
                    ::Intersection hit{}; 
                    Color color = background;

                    if (intersect(ray, hit))
                        color = shade(hit, camera);

                    framebuffer(x, y).set(clampColor(color));
                }
            }
        };

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i)
    {
        int y0 = i * linesPerThread;
        int y1 = (i == numThreads - 1) ? H : y0 + linesPerThread;
        threads.emplace_back(renderBlock, y0, y1);
    }

    for (auto& t : threads)
        if (t.joinable()) t.join();

    if (!cancelFlag.load())
        image.setData(framebuffer);
}