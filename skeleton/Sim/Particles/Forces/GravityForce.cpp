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

void GravityForce::Apply(physx::PxRigidDynamic* rb, double dt)
{
	PX_UNUSED(dt);
	if (!Enabled() || rb == nullptr) return;

	const float m = rb->getMass();
	const Vector3D F = _gravity * m; // g (m/s^2) -> F = m*g (N)

	rb->addForce(physx::PxVec3(F.getX(), F.getY(), F.getZ()));
}
