#ifndef UNIFORM_PARTICLE_GENERATOR_H_
#define UNIFORM_PARTICLE_GENERATOR_H_

#include "ParticleGenerator.h"

class UniformParticleGenerator : public ParticleGenerator
{
private:
	Vector3 vel_width, pos_width;

public:
	// std::list<Particle*> generateParticles();
};

#endif /*UNIFORM_PARTICLE_GENERATOR_H_*/