#pragma once
#include <limits>
#include <algorithm>
#include <cmath>

#include <core/List.h>
#include <graphics/Light.h>
#include <graphics/Image.h>
#include <graphics/Camera.h>
#include <graphics/Color.h>

#include "Actor.h"
#include "Intersection.h"

using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

class Scene : public SharedObject
{
public:
	// --- Objetos da Cena ---
	List<Reference<Actor>> _actors;
	List<Reference<Light>> _lights;

	// --- Configurações ---
	Color background{ 0.05f, 0.05f, 0.05f };
	Color ambientLight{ 0.1f, 0.1f, 0.1f };

	void addActor(Actor* a) { _actors.add(a); }
	void addLight(Light* l) { _lights.add(l); }

	bool intersect(const Ray3<float>& ray, Intersection& hit) const;
	Color shade(const Intersection& hit, const Camera& camera) const;
	void render(const Camera& camera, Image& image) const;
};