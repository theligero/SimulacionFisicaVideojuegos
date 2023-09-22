#include "Particle.h"

Particle::Particle(Vector3 _pos, Vector3 _vel)
{
	pos = PxTransform(_pos.x, _pos.y, _pos.z);
	vel = _vel;
	renderItem = new RenderItem(CreateShape(PxSphereGeometry(1.5)),	&pos, {0, 0, 0.5, 1.0});
}

Particle::~Particle()
{
	DeregisterRenderItem(renderItem);
}

void Particle::update(double t)
{
	pos = PxTransform(pos.p + vel * t);
}
