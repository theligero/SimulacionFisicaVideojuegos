#include "Particle.h"
#include <iostream>

Particle::Particle(PxTransform _tr, Vector3 _vel, Vector3 _accel, Vector4 _color, float d, float m)
{
	tr = _tr;
	color = _color;
	accel = _accel;
	vel = _vel;
	damping = d;
	inv_mass = 1.0f / m;
	renderItem = new RenderItem(CreateShape(PxSphereGeometry(1.5)), &tr, color);
}

Particle::~Particle()
{
	DeregisterRenderItem(renderItem);
	delete renderItem;
}

void Particle::integrate(double t)
{
	if (inv_mass <= 0.0f) return;

	tr.p += vel * t;

	vel += accel * t;

	vel *= powf(damping, t);
}

void Particle::setMass(float m)
{
	inv_mass = 1.0f / m;
}

void Particle::setVelocity(Vector3 v)
{
	vel = v;
}

void Particle::setAcceleration(Vector3 a)
{
	accel = a;
}

void Particle::setDamping(float d)
{
	damping = d;
}

void Particle::setPosition(Vector3 p)
{
	tr.p = p;
}
