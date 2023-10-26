#ifndef FIREWORK_H_
#define FIREWORK_H_

#include "ParticleGenerator.h"
#include <memory>

class Firework : public Particle
{
protected:
	unsigned type;
	std::list<std::shared_ptr<ParticleGenerator>> gens;
public:
	std::list<Particle*> explode();
	void addGenerator(ParticleGenerator* p);
	virtual Particle* clone() const;
};

#endif FIREWORK_H_