#ifndef GRAVITY_FORCE_H_
#define GRAVITY_FORCE_H_

#pragma once

#include "ForceGenerator.h"

class GravityForce : public ForceGenerator
{
private:
	Vector3D _gravity;
public:
	explicit GravityForce(const Vector3D& g);
	void SetGravity(const Vector3D& g);

	void Apply(Particle& p, double dt) override;
};

#endif // GRAVITY_FORCE_H_