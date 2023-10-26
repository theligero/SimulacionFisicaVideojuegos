#ifndef GAUSSIAN_PARTICLE_GENERATOR_H_
#define GAUSSIAN_PARTICLE_GENERATOR_H_

#include "ParticleGenerator.h"

class GaussianParticleGenerator : public ParticleGenerator
{
protected:
	Vector3 std_dev_pos, std_dev_vel;
	double std_dev_t;

	std::normal_distribution<double> d{ 0,1 };

public:
	GaussianParticleGenerator(std::string name, int part, Vector3 pos, Vector3 vel, double t, Particle* m) :
		std_dev_pos(pos), std_dev_vel(vel), std_dev_t(t), ParticleGenerator(name, part, m) {}
	virtual std::list<Particle*> generateParticles() override;
};

#endif /*GAUSSIAN_PARTICLE_GENERATOR_H_*/