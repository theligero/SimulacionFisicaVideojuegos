#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "../Math/Vector3D.h"
#include "../../RenderUtils.hpp"

class Particle {
private:
	Vector3D _vel;
	Vector3D _grav;
	physx::PxTransform _pose;
	RenderItem* _renderItem = nullptr;
	physx::PxShape* _shape = nullptr;

	float _radius = 0.2f;
	float _damping = 1.0f;
	double _life = -1.0;
	double _age = 0.0;
	float _mass = 1.0f;

	Vector3D _forceAcc{ 0, 0, 0 };

	static inline physx::PxVec3 toPx(const Vector3D& v) { return physx::PxVec3(v.getX(), v.getY(), v.getZ()); }
	static inline Vector3D fromPx(const physx::PxVec3& v) { return Vector3D{ v.x, v.y, v.z }; }
public:
	Particle(const Vector3D& pos, const Vector3D& vel, float radius = 0.1f,
		const Vector4& color = Vector4(1,1,1,1), double lifeSeconds = -1.0,
		float damping = 1.0f, const Vector3D& gravity = Vector3D{0, -9.8, 0},
		float mass = 1.0f);
	~Particle();

	void Integrate(double dt);
	bool IsAlive() const;

	void SetGravity(const Vector3D& v);

	Vector3D GetPosition() const { return Vector3D{ _pose.p.x, _pose.p.y, _pose.p.z }; };

	float GetMass() const;
	Vector3D GetVelocity() const;
	physx::PxVec3 GetPositionPx() const;
	void AddForce(const Vector3D& f);
	void ClearForces();
};

#endif // PARTICLE_H_