#include "Particle.h"
#include <iostream>

Particle::Particle(PxTransform _tr, Vector3 _vel, Vector3 _accel, Vector4 _color, float damp, float m)
{
	tr = _tr;
	color = _color;
	accel = _accel;
	vel = _vel;
	damping = damp;
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
	if (duration > 0) {
		if (inv_mass <= 0.0f) return;

		tr.p += vel * t;

		vel += accel * t;

		vel *= powf(damping, t);

		duration -= t;
	}
	else delete this;
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

void Particle::setDuration(double t)
{
	duration = t;
}

void Particle::setIterator(std::list<Particle*>::iterator iterator)
{
	it = iterator;
}
