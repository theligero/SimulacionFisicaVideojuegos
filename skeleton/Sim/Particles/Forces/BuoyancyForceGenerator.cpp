#include "BuoyancyForceGenerator.h"

BuoyancyForceGenerator::BuoyancyForceGenerator(Particle* object, Particle* liquid,
	float height, float volume, float density) : _object(object), _liquid(liquid),
	_height(height), _volume(volume), _density(density), _gravity(9.8f)
{

}

void BuoyancyForceGenerator::Apply(Particle& p, double dt)
{
	if (!Enabled()) return;

	// Solo queremos aplicar el empuje a la partícula de flotación
	if (_object && &p != _object) return;
	if (!_object || !_liquid) return;

	const float yObj = _object->GetPosition().getY();
	const float yLiq = _liquid->GetPosition().getY();

	// Calculamos cuánto está sumergido (0 = nada, 1 = totalmente sumergido)
	float immersedFraction = 0.0f;

	// Centro del objeto demasiado por encimad el agua -> no hay empuje
	if (yObj - yLiq >= _height * 0.5f) immersedFraction = 0.0f;

	// Centro por debajo del todo -> completamente sumergido
	else if (yLiq - yObj >= _height * 0.5f) immersedFraction = 1.0f;

	else {
		// Fracción sumergida lineal entre -h/2 y h/2
		// cuando yObj = yLiq -> 1/2 sumergido
		immersedFraction = (yLiq - yObj) / _height + 0.5f;
		if (immersedFraction < 0.0f) immersedFraction = 0.0f;
		if (immersedFraction > 1.0f) immersedFraction = 1.0f;
	}

	if (immersedFraction <= 0.0f) return;

	float displacedVolume = _volume * immersedFraction;
	float buoyantForceMag = _density * displacedVolume * _gravity;

	Vector3D force(0.0f, buoyantForceMag, 0.0f);
	_object->AddForce(force);
}

void BuoyancyForceGenerator::SetObject(Particle* p)
{
	_object = p;
}

void BuoyancyForceGenerator::SetLiquid(Particle* p)
{
	_liquid = p;
}

void BuoyancyForceGenerator::SetHeight(float h)
{
	_height = h;
}

void BuoyancyForceGenerator::SetVolume(float v)
{
	_volume = v;
}

void BuoyancyForceGenerator::SetDensity(float d)
{
	_density = d;
}

float BuoyancyForceGenerator::GetVolume() const
{
	return _volume;
}
