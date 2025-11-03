#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_

#include "Emitter.h"

class ParticleSystem
{
private:
	bool Outside(const Vector3D& p) const;

	std::vector<std::unique_ptr<IEmitter>> _emitters;
	std::vector<std::unique_ptr<Particle>> _particles;

	Vector3D _gravity{ 0, -10, 0 };
	bool _gravitySet = false;

	AABB _bounds{ Vector3D{-50,-50,-50}, Vector3D{50,50,50} };
	bool _useBounds = false;

public:
	ParticleSystem() = default;
	~ParticleSystem();

	void AddEmitter(std::unique_ptr<IEmitter> e);

	void SetGravityAll(const Vector3D& g);
	void SetBounds(const AABB& aabb);

	void Update(double dt);
	void Clear();

	size_t Count() const;
};

#endif // PARTICLE_SYSTEM_H_