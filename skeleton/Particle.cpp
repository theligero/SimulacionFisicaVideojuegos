#include "Particle.h"
#include <iostream>

Particle::Particle(Vector3 _pos, Vector3 _vel, Vector3 _accel, float d)
{
	pos = physx::PxTransform({ _pos.x, _pos.y, _pos.z });
	accel = _accel;
	vel = _vel;
	damping = d;
	renderItem = new RenderItem(CreateShape(physx::PxSphereGeometry(1.5)), &pos, { 0, 0, 0.5, 1.0 });
}

Particle::~Particle()
{
	DeregisterRenderItem(renderItem);
	delete renderItem;
}

void Particle::integrate(double t)
{
	pos.p += vel * t;

	vel += accel * t;

	vel *= powf(damping, t);

	// std::cout << vel.x << ", " << vel.y << ", " << vel.z << std::endl;
}
