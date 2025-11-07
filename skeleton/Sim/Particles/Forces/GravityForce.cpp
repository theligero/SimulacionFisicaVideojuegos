#include "GravityForce.h"

GravityForce::GravityForce(const Vector3D& g) : 
	_gravity(g) {

}

void GravityForce::SetGravity(const Vector3D& g) {
	_gravity = g;
}

void GravityForce::Apply(Particle& p, double dt)
{
	if (!Enabled()) return;
	p.AddForce(_gravity * p.GetMass());
}
