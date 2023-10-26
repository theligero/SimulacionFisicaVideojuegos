#include "ParticleSystem.h"

ParticleSystem::~ParticleSystem()
{
	particles_generator.clear();
	for (auto it = particles.begin(); it != particles.end(); ++it) {
		particles.erase(it);
	}
}

void ParticleSystem::update(double t)
{
	for (auto it = particles.begin(); it != particles.end(); ++it) {
		if ((*it) != nullptr) (*it)->integrate(t);
		else particles.erase(it);
	}
}

ParticleGenerator* ParticleSystem::getParticleGenerator(std::string name)
{
	if (particles_generator.count(name)) return particles_generator[name];
	else return nullptr;
}

void ParticleSystem::generateFireworkSystem(unsigned firework_type)
{

}

void ParticleSystem::generateSimple()
{

}
