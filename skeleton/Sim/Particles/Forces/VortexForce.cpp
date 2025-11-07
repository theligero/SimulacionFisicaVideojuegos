#include "VortexForce.h"

VortexForce::VortexForce(const Vector3D& center, float K, float k1, float k2) :
	WindForce(Vector3D{0, 0, 0}, k1, k2), _C(center), _K(K)
{
}

Vector3D VortexForce::WindAt(const Vector3D& pos) const
{
	const float dx = pos.getX() - _C.getX();
	const float dz = pos.getZ() - _C.getZ();
	return Vector3D{ -_K * dz, 0.0f, _K * dx };
}
