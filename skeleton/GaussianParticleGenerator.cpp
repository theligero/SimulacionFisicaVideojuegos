#include "GaussianParticleGenerator.h"

std::list<Particle*> GaussianParticleGenerator::generateParticles()
{
	std::list<Particle*> lista;
	Particle* aux = nullptr;

	for (int i = 0; i < num_particles; ++i) {
		PxTransform tr = PxTransform(d(std_dev_pos.x), d(std_dev_pos.y), d(std_dev_pos.z));
		Vector3 vel = Vector3(d(std_dev_vel.x), d(std_dev_vel.y), d(std_dev_vel.z));
		// aux = new Particle(tr, vel, { 0, -9.8, 0 }, )
		lista.push_back(aux);
	}

	return lista;
}
