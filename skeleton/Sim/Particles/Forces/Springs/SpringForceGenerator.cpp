#include "SpringForceGenerator.h"

SpringForceGenerator::SpringForceGenerator(float k, float restingLength,
	Particle* particle, Particle* other) :
	_k(k), _restLength(restingLength), _particle(particle), _other(other) 
{
}

void SpringForceGenerator::Apply(Particle& p, double dt)
{
	if (!Enabled()) return;

	// Verificar si p es alguna de las dos partículas del muelle
	bool isPrimary = (_particle == &p);
	bool isOther = (_other == &p);

	if (!isPrimary && !isOther) return; // No es ninguna de las dos

	Vector3D relativePos;
	if (isPrimary) {
		// Vector desde Other hacia Particle
		relativePos = _particle->GetPosition() - _other->GetPosition();
	}
	else {
		// Vector desde Particle hacia Other (si smos la 'otra', calculamos al revés)
		relativePos = _other->GetPosition() - _particle->GetPosition();
	}

	float length = relativePos.length();
	if (length == 0.0f) return;

	// F = -k * (|L| - L0) * u
	float x = length - _restLength;
	Vector3D dir = relativePos * (1.0f / length);

	// Si somos la partícula primaria, la fuerza es -k*x
	// Si somos la secundaria, la lógica del vector relativo ya ajustó la dirección

	// Calculamos vector A -> B siempre
	Vector3D diff = _other->GetPosition() - _particle->GetPosition();
	float dist = diff.length();
	if (dist == 0.0f) return;

	float expansion = dist - _restLength;
	Vector3D forceDirection = diff * (1.0f / dist); // Dirección unitaria de A a B

	// Fuerza magnitud: F = k * expansion
	Vector3D force = forceDirection * (_k * expansion);

	// Si p es A (primary), la fuerza debe atraerlo hacia B -> dirección positiva
	// Hooke: F = -k * x. Si x > 0 (estirado), la fuerza debe acercarlos

	if (isPrimary) {
		p.AddForce(force);
	}
	else {
		p.AddForce(force * -1.0f); // A la otra partícula, fuerza opuesta
	}
}

void SpringForceGenerator::SetK(float k)
{
	_k = k;
}

void SpringForceGenerator::SetRestLength(float l)
{
	_restLength = l;
}

void SpringForceGenerator::SetParticle(Particle* p)
{
	_particle = p;
}

void SpringForceGenerator::SetOther(Particle* other)
{
	_other = other;
}