#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "../Math/Vector3D.h"
#include "../../RenderUtils.hpp"

class Particle {
private:
	Vector3D _vel;
	physx::PxTransform _pose;
	RenderItem* _renderItem = nullptr;
	physx::PxShape* _shape = nullptr;

	float _radius = 0.2f;
	float _damping = 1.0f;
	double _life = -1.0;
	double _age = 0.0;

	static inline physx::PxVec3 toPx(const Vector3D& v) { return physx::PxVec3(v.getX(), v.getY(), v.getZ()); }
	static inline Vector3D fromPx(const physx::PxVec3& v) { return Vector3D{ v.x, v.y, v.z }; }
public:
	Particle(const Vector3D& pos, const Vector3D& vel, float radius = 0.2f,
		const Vector4& color = Vector4(1,1,1,1), double lifeSeconds = -1.0,
		float damping = 1.0f);
	~Particle();

	void Integrate(double dt);
	bool IsAlive() const;
};

#endif // PARTICLE_H_