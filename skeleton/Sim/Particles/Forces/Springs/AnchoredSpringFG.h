#ifndef ANCHORED_SPRING_FG_H
#define ANCHORED_SPRING_FG_H

#include "../ForceGenerator.h"

class AnchoredSpringFG : public ForceGenerator {
public:
	/// <summary>
	/// Constructora por defecto
	/// </summary>
	/// <param name="k">Constante elástica</param>
	/// <param name="restingLength">Longitud en reposo</param>
	/// <param name="anchorPos">Posición fija del ancla</param>
	/// <param name="particle">Partícula sobre la que se aplica el muelle</param>
	AnchoredSpringFG(float k, float restingLength, 
		const Vector3D& anchorPos,
		Particle* particle = nullptr);
	
	void Apply(Particle& p, double dt) override;

	void SetParticle(Particle* p);
	void SetK(float k);
	void SetRestLength(float l);
	void SetAnchor(const Vector3D& pos);

	float GetK() const;
private:
	float _k;
	float _restLength;
	Vector3D _anchor;
	Particle* _particle;
};

#endif // ANCHORED_SPRING_FG_H_