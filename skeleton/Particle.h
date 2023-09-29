#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "RenderUtils.hpp"

class Particle
{
private:
	physx::PxTransform pos;
	Vector3 vel;
	RenderItem* renderItem;

	Vector3 accel;
	float damping;
public:
	Particle(Vector3 _pos, Vector3 _vel, Vector3 _accel, float d);
	~Particle();

	void integrate(double t);
};

#endif /*PARTICLE_H_*/