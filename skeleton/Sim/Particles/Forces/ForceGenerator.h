#ifndef FORCE_GENERATOR_H_
#define FORCE_GENERATOR_H_

#pragma once

#include "../Particle.h"

class ForceGenerator
{
protected:
	bool _enabled = true;
public:
	virtual ~ForceGenerator() = default;
	void SetEnabled(bool e);
	bool Enabled() const;

	// Aplica F a la partícula (usando addForce)
	virtual void Apply(Particle& p, double dt) = 0;
};

#endif // FORCE_GENERATOR_H_