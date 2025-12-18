#ifndef BUOYANCY_FORCE_GENERATOR_H_
#define BUOYANCY_FORCE_GENERATOR_H_

#include "ForceGenerator.h"

class BuoyancyForceGenerator : public ForceGenerator {
public:
	/// <summary>
	/// Constructora por defecto
	/// </summary>
	/// <param name="object">Partícula que flota</param>
	/// <param name="liquid">Partícula que marca el nivel del líquido</param>
	/// <param name="height">Altura del objeto sumergible (dimensión vertical característica)</param>
	/// <param name="volume">Volumen del objeto</param>
	/// <param name="density">Densidad del líquido (por defecto agua)</param>
	BuoyancyForceGenerator(Particle* object, Particle* liquid,
		float height, float volume, float density = 1000.0f);

	void Apply(Particle& p, double dt) override;

	void SetObject(Particle* p);
	void SetLiquid(Particle* p);

	void SetHeight(float h);
	void SetVolume(float v);
	void SetDensity(float d);

	float GetVolume() const;

private:
	Particle* _object = nullptr; // objeto de flotación
	Particle* _liquid = nullptr; // referencia para nivel de líquido

	float _height; // altura del objeto (tamaño vertical)
	float _volume; // volumen del objeto
	float _density; // densidad del líquido
	float _gravity; // módulo de g (9.8)
};

#endif // BUOYANCY_FORCE_GENERATOR_H_