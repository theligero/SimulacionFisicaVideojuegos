#ifndef EXPLOSION_FORCE_H_
#define EXPLOSION_FORCE_H_

#pragma once
#include "ForceGenerator.h"

class ExplosionForce : public ForceGenerator
{
private:
	Vector3D _C;
	float _K, _R, _tau;
	double _t = 0.0;
	bool _active = false;

public:
	ExplosionForce(const Vector3D& center, float K, float R, float tau);

	void Reset();
	void Stop();
	bool Active() const;

	void Apply(Particle& p, double dt) override;
};

#endif // EXPLOSION_FORCE_H_