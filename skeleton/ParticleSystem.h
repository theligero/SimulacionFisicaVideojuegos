#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_

#include "GaussianParticleGenerator.h"
#include <unordered_map>

class ParticleSystem
{
protected:
	std::list<Particle*> particles;
	std::unordered_map<std::string, ParticleGenerator*> particles_generator;
	// ParticleGenerator* firework_generator;
	Vector3 gravity;

public:
	ParticleSystem() {}
	~ParticleSystem();

	void update(double t);
	ParticleGenerator* getParticleGenerator(std::string name);
	void generateFireworkSystem(unsigned firework_type);
	void generateSimple();
};

#endif /*PARTICLE_SYSTEM_H_*/