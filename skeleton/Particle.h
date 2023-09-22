#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "RenderUtils.hpp"

using namespace physx;

class Particle
{
private:
	PxTransform pos;
	Vector3 vel;
	RenderItem* renderItem;
public:
	Particle(Vector3 _pos, Vector3 _vel);
	~Particle();
	void update(double t);
};

#endif /*PARTICLE_H_*/