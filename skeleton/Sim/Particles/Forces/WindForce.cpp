#include "WindForce.h"

#include <cmath>
#include <iostream>

bool WindForce::Inside(const Vector3D& x) const
{
	if (!_useVol) return false;
	return (x.getX() >= _vol.min.getX() && x.getY() >= _vol.min.getY() && x.getZ() >= _vol.min.getZ() &&
		x.getX() <= _vol.max.getX() && x.getY() <= _vol.max.getY() && x.getZ() <= _vol.max.getZ());
}

Vector3D WindForce::WindAt(const Vector3D& pos) const
{
	return _wind;
}

WindForce::WindForce(const Vector3D& windVel, float k1, float k2) :
	_wind(windVel), _k1(k1), _k2(k2)
{
}

void WindForce::SetVolume(const AABB& aabb)
{
	_useVol = true;
	_vol = aabb;
}

void WindForce::ClearVolume()
{
	_useVol = false;
}

void WindForce::SetWind(const Vector3D& v)
{
	_wind = v;
}

void WindForce::SetCoeffs(float k1, float k2)
{
	_k1 = k1; _k2 = k2;
}

void WindForce::SetK1(float k1)
{
	_k1 = k1;
}

void WindForce::SetK2(float k2)
{
	_k2 = k2;
}

float WindForce::GetK1() const
{
	return _k1;
}

float WindForce::GetK2() const
{
	return _k2;
}

void WindForce::Apply(Particle& p, double dt)
{
	if (!Enabled()) return;
	const Vector3D pos = p.GetPosition();
	if (_useVol && !Inside(pos)) return;

	const Vector3D v = p.GetVelocity();
	const Vector3D vw = WindAt(pos);
	const Vector3D vrel = vw - v;


	// F = k1 * vrel + k2 * |vrel| * vrel
	const float speed = std::sqrt(vrel.getX() * vrel.getX() + vrel.getY() * 
		vrel.getY() + vrel.getZ() * vrel.getZ());
	Vector3D F = vrel * _k1;
	if (_k2 != 0.0f) F = F + vrel * (_k2 * speed);

	p.AddForce(F);
}

void WindForce::Apply(physx::PxRigidDynamic* rb, double dt)
{
	if (!Enabled()) return;

	// Obtenemos posición y velocidad de PhysX
	physx::PxTransform t = rb->getGlobalPose();
	Vector3D pos(t.p.x, t.p.y, t.p.z);

	if (_useVol && !Inside(pos)) return;

	physx::PxVec3 velPx = rb->getLinearVelocity();
	Vector3D v(velPx.x, velPx.y, velPx.z);

	// Calculamos fuerza igual que con partículas
	Vector3D vw = WindAt(pos);
	Vector3D vrel = vw - v;
	float speed = vrel.length();

	// F = k1 * vrel + k2 * |vrel| * vrel
	Vector3D F = vrel * _k1;
	if (_k2 != 0.0f) F = F + vrel * (_k2 * speed);

	// Aplicamos la fuerza a PhysX
	rb->addForce(physx::PxVec3(F.getX(), F.getY(), F.getZ()));
}
