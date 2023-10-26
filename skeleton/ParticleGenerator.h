#ifndef PARTICLE_GENERATOR_H_
#define PARTICLE_GENERATOR_H_

#include "Particle.h"
#include <random>

class ParticleGenerator
{
protected:
	Vector3 mean_pos, mean_vel;
	double gen_prob = 1.0;
	int num_particles;
	Particle* model = nullptr;

	std::mt19937 mt;
	std::uniform_real_distribution<double> u{ 0,1 };
	std::string name;
public:
	ParticleGenerator(std::string n, int part, Particle* m) : 
		name(n), num_particles(part) {}
	~ParticleGenerator();
	void update(double t);

	virtual std::list<Particle*> generateParticles() = 0;
	inline void setOrigin(const Vector3& p) { mean_pos = p; }
	inline void setMeanVelocity(const Vector3& v) { mean_vel = v; }
	inline Vector3 getMeanVelocity() const { return mean_vel; }
	inline void setMeanDuration(double new_duration) { model->setDuration(new_duration); }
	inline void setParticle(Particle* p, bool modify_pos_vel = true) { 
		// delete model; 
		// model = p->clone();
		// if (modify_pos_vel) {
			// mean_pos = p->getPosition();
		// }
	}
	inline void setNParticles(const int& n_p) { num_particles = n_p; }
};

#endif /*PARTICLE_GENERATOR_H_*/