#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "RenderUtils.hpp"
#include <list>

using namespace physx;

class Particle
{
private:
	PxTransform tr; // transform
	Vector3 vel; // velocidad
	RenderItem* renderItem; // objeto que se renderiza

	Vector3 accel; // aceleración
	float damping; // amortiguamiento

	float inv_mass; // inverso de la masa
	Vector4 color; // color de la partícula
	double duration = 10; // tiempo de vida
	std::list<Particle*>::iterator it; // iterador de posición de la partícula en la lista
public:
	Particle(PxTransform _tr, Vector3 _vel, Vector3 _accel, Vector4 _color, float damp, float m);
	~Particle();

	void integrate(double t);

	void setMass(float m);
	void setVelocity(Vector3 v);
	void setAcceleration(Vector3 a);
	void setDamping(float d);
	void setPosition(Vector3 p);
	void setDuration(double t);
	void setIterator(std::list<Particle*>::iterator iterator);
};

#endif /*PARTICLE_H_*/