#ifndef VORTEX_FORCE_H_
#define VORTEX_FORCE_H_

#pragma once
#include "WindForce.h"

class VortexForce : public WindForce
{
private:
	Vector3D _C;
	float _K;

protected:
	Vector3D WindAt(const Vector3D& pos) const override;

public:
	VortexForce(const Vector3D& center, float K, float k1 = 1.0f, float k2 = 0.0f);
};

#endif // VORTEX_FORCE_H_