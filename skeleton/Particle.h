#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "RenderUtils.hpp"

using namespace physx;

class Particle
{
private:
	PxTransform tr;
	Vector3 vel;
	RenderItem* renderItem;

	Vector3 accel;
	float damping;

	float inv_mass;
	Vector4 color;
public:
	Particle(PxTransform _tr, Vector3 _vel, Vector3 _accel, Vector4 _color, float d, float m);
	~Particle();

	void integrate(double t);

	void setMass(float m);
	void setVelocity(Vector3 v);
	void setAcceleration(Vector3 a);
	void setDamping(float d);
	void setPosition(Vector3 p);
};

#endif /*PARTICLE_H_*/