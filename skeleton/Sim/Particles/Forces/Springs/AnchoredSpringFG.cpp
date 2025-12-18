#include "AnchoredSpringFG.h"

AnchoredSpringFG::AnchoredSpringFG(float k, float restingLength,
	const Vector3D& anchorPos, Particle* particle) :
	_k(k), _restLength(restingLength), _anchor(anchorPos), _particle(particle)
{

}

void AnchoredSpringFG::Apply(Particle& p, double dt)
{
	if (!Enabled()) return;

	// Solo actuamos sobre la partícula asociada (si se ha especificado)
	if (_particle && &p != _particle) return;

	// Vector desde el ancla hasta la partícula
	Vector3D rel = p.GetPosition() - _anchor;
	float length = rel.length();
	if (length == 0.0f) return;

	float x = length - _restLength;
	Vector3D dir = rel * (1.0f / length);

	Vector3D force = dir * (-_k * x);
	p.AddForce(force);
}

void AnchoredSpringFG::SetK(float k)
{
	_k = k;
}

void AnchoredSpringFG::SetRestLength(float l)
{
	_restLength = l;
}

void AnchoredSpringFG::SetAnchor(const Vector3D& pos)
{
	_anchor = pos;
}

float AnchoredSpringFG::GetK() const
{
	return _k;
}

void AnchoredSpringFG::SetParticle(Particle* p)
{
	_particle = p;
}
