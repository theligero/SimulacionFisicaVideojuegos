#ifndef WIND_FORCE_H_
#define WIND_FORCE_H_

#pragma once
#include "ForceGenerator.h"
#include "../../Utils/AABB.h"

class WindForce : public ForceGenerator
{
private:
	bool Inside(const Vector3D& x) const;

	Vector3D _wind;
	float _k1, _k2;
	bool _useVol = false;
	AABB _vol;

protected:
	virtual Vector3D WindAt(const Vector3D& pos) const;

public:
	WindForce(const Vector3D& windVel, float k1 = 1.0f, float k2 = 0.0f);

	void SetVolume(const AABB& aabb);
	void ClearVolume();

	void SetWind(const Vector3D& v);
	void SetCoeffs(float k1, float k2);

	void SetK1(float k1);
	void SetK2(float k2);

	float GetK1() const;
	float GetK2() const;

	void Apply(Particle& p, double dt) override;
};

#endif // WIND_FORCE_H_