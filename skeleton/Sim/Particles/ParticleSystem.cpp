#include "ParticleSystem.h"

#include "Particle.h"

#include <iostream>

bool ParticleSystem::Outside(const Vector3D& p) const {
	if (!_useBounds) return false;
	return p.getX() < _bounds.min.getX() || p.getY() < _bounds.min.getY() || p.getZ() < _bounds.min.getZ() ||
		p.getX() > _bounds.max.getX() || p.getY() > _bounds.max.getY() || p.getZ() > _bounds.max.getZ();
}

ParticleSystem::~ParticleSystem() {
	Clear();
}

void ParticleSystem::AddEmitter(std::unique_ptr<IEmitter> e) {
	_emitters.emplace_back(std::move(e));
}

void ParticleSystem::SetGravityAll(const Vector3D& g) {
	_gravity = g;
	_gravitySet = true;
}

void ParticleSystem::SetBounds(const AABB& aabb) {
	_bounds = aabb;
	_useBounds = true;
}

void ParticleSystem::Update(double dt) {
	// Pedir a cada emisor nuevas partículas
	for (auto& e : _emitters) e->Emit(dt, _particles);

	// Aplicar generadores de fuerza
	for (auto& p : _particles) if (p) p->ClearForces();
	for (auto& fg : _forces) {
		for (auto& p : _particles) if (p) fg->Apply(*p, dt);
	}

	// Integrar
	for (auto& p : _particles) if (p) p->Integrate(dt);

	// Limpieza: vida vencida o fuera de límites
	_particles.erase(
		std::remove_if(_particles.begin(), _particles.end(),
			[&](const std::unique_ptr<Particle>& p) {
				if (!p) return true;
				const Vector3D pos = p->GetPosition();
				if (p->IsAlive() && !Outside(pos)) return false;
				return true; // borra -> destructora limpia RenderItem / Shape
			}),
		_particles.end());
}

void ParticleSystem::Clear() {
	_emitters.clear();
	_particles.clear();
}

size_t ParticleSystem::Count() const {
	return _particles.size();
}

void ParticleSystem::AddForceGenerator(std::unique_ptr<ForceGenerator> f)
{
	_forces.emplace_back(std::move(f));
}

std::vector<std::unique_ptr<ForceGenerator>>& ParticleSystem::Forces()
{
	return _forces;
}
