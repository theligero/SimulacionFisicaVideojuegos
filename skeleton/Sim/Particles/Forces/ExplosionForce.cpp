#include "ExplosionForce.h"

#include <cmath>

ExplosionForce::ExplosionForce(const Vector3D& center, float K, float R, float tau) :
	_C(center), _K(K), _R(R), _tau(tau)
{
}

void ExplosionForce::Reset()
{
	_t = 0.0; _active = true;
}

void ExplosionForce::Stop()
{
	_active = false;
}

bool ExplosionForce::Active() const
{
	return _active;
}

void ExplosionForce::Apply(Particle& p, double dt)
{
	if (!Enabled() || !_active) return;
	_t += dt;
	if (_t > 4.0 * _tau) {
		_active = false; return;
	}

	const Vector3D pos = p.GetPosition();
	const float dx = pos.getX() - _C.getX(),
				dy = pos.getY() - _C.getY(),
				dz = pos.getZ() - _C.getZ();
	const float r2 = dx * dx + dy * dy + dz * dz;
	const float r = std::sqrt(std::max<float>(1e-6f, r2));
	if (r > _R) return;

	// F = K / r^2 * e^{-t/tau}
	const float scale = (_K / r2) * std::exp(float(-_t / _tau));
	p.AddForce(Vector3D{ dx * scale, dy * scale, dz * scale });
}
