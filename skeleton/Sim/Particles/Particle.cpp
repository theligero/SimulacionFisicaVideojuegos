#include "Particle.h"

#include <cmath>

Particle::Particle(const Vector3D& pos, const Vector3D& vel, 
	float radius, const Vector4& color, double lifeSeconds, float damping) :
	_vel(vel), _pose(physx::PxTransform(toPx(pos))), _radius(radius), _damping(damping),
	_life(lifeSeconds)
{
	// Crear shape
	_shape = CreateShape(physx::PxSphereGeometry(_radius));

	// Crear y registrar el item de renderizado que apunta a _pose
	_renderItem = new RenderItem(_shape, &_pose, color);
	RegisterRenderItem(_renderItem);
}

Particle::~Particle()
{
	// Quitar del registro y limpiar en orden inverso
	if (_renderItem) {
		DeregisterRenderItem(_renderItem);
		delete _renderItem;
		_renderItem = nullptr;
	}
	if (_shape) {
		_shape->release();
		_shape = nullptr;
	}
}

void Particle::Integrate(double dt)
{
	if (dt <= 0.0) return;
	_age += dt;
	if (!IsAlive()) return;

	// Damping "por segundo": elevar a dt para que sea independiente del frame
	if (_damping != 1.0f) {
		float factor = std::pow(_damping, static_cast<float>(dt));
		_vel = _vel * factor;
	}

	// x_{x+dt} = x_t + v_{t+dt} * dt (semi-implícito)
	physx::PxVec3 v = toPx(_vel);
	_pose.p += v * static_cast<float>(dt);
}

bool Particle::IsAlive() const
{
	return _life < 0.0 || _age < _life;
}
