#include "Particle.h"

#include <cmath>

Particle::Particle(const Vector3D& pos, const Vector3D& vel, 
	float radius, const Vector4& color, double lifeSeconds, float damping, 
	const Vector3D& gravity, float mass) :
	_vel(vel), _pose(physx::PxTransform(toPx(pos))), _radius(radius), _damping(damping),
	_life(lifeSeconds), _grav(gravity), _mass(mass)
{
	// Crear shape
	_shape = CreateShape(physx::PxSphereGeometry(_radius));

	// Crear y registrar el item de renderizado que apunta a _pose
	_renderItem = new RenderItem(_shape, &_pose, color);
}

Particle::~Particle()
{
	// Quitar del registro y limpiar en orden inverso
	if (_renderItem) {
		_renderItem->release();
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

	Vector3D a = ((_mass > 0.f) ? _forceAcc * (1.0f / _mass) : Vector3D{ 0, 0, 0 });
	_vel = _vel + a * float(dt);

	// Damping "por segundo": elevar a dt para que sea independiente del frame
	if (_damping != 1.0f) {
		float factor = std::pow(_damping, static_cast<float>(dt));
		_vel = _vel * factor;
	}

	// x_{x+dt} = x_t + v_{t+dt} * dt
	_pose.p += toPx(_vel) * float(dt);

	_forceAcc = Vector3D{ 0, 0, 0 };
}

bool Particle::IsAlive() const
{
	return _life < 0.0 || _age < _life;
}

void Particle::SetGravity(const Vector3D& v)
{
	_grav = v;
}

float Particle::GetMass() const
{
	return _mass;
}

Vector3D Particle::GetVelocity() const
{
	return _vel;
}

physx::PxVec3 Particle::GetPositionPx() const
{
	return _pose.p;
}

void Particle::AddForce(const Vector3D& f) {
	_forceAcc += f;
}

void Particle::ClearForces()
{
	_forceAcc = Vector3D{ 0, 0, 0 };
}
