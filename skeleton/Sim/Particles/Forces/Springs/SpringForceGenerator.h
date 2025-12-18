#ifndef SPRING_FORCE_GENERATOR_H_
#define SPRING_FORCE_GENERATOR_H_

#include "../ForceGenerator.h"

class SpringForceGenerator : public ForceGenerator {
public:
	/// <summary>
	/// Constructora por defecto
	/// </summary>
	/// <param name="k">Constante elástica</param>
	/// <param name="restingLength">Longitud en reposo</param>
	/// <param name="particle">Partícula sobre la que se aplica la fuerza del muelle</param>
	/// <param name="other">Otra partícula que hace de "extremo" del muelle</param>
	SpringForceGenerator(float k, float restingLength,
		Particle* particle = nullptr,
		Particle* other = nullptr);

	void Apply(Particle& p, double dt) override;

	void SetK(float k);
	void SetRestLength(float l);
	void SetParticle(Particle* p);
	void SetOther(Particle* other);

private:
	float _k;
	float _restLength;
	Particle* _particle;
	Particle* _other;
};

#endif // SPRING_FORCE_GENERATOR_H_