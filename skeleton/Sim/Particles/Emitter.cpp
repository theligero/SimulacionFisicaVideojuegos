#include "Emitter.h"
#include "Particle.h"

#include <algorithm>

// Helpers
static inline Vector3D ToV3(const physx::PxVec3& v) { return Vector3D{ v.x, v.y, v.z }; }
static inline physx::PxVec3 ToPx(const Vector3D& v) { return physx::PxVec3(v.getX(), v.getY(), v.getZ()); }

// ...
static Vector3D RandomUnit(RNG& R) {
	std::uniform_real_distribution<float> U(-1.f, 1.f), A(0.f, 6.2831853f);
	float z = U(R.rng);
	float t = A(R.rng);
	float r = std::sqrt(std::max<float>(0.f, 1.f - z * z));
	return Vector3D{ r * std::cos(t), r * std::sin(t), z };
}

Vector3D PointEmitter::RandomDirInCone(RNG& R) const
{
	if (_halfAngle <= 0.f) return _dir;

	// TO-DO
	std::uniform_real_distribution<float> U(0.f, 1.f);
	float u = U(R.rng);
	float v = U(R.rng);
	float cosTheta = 1.f - u * (1.f - std::cos(_halfAngle));
	float sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
	float phi = 6.2831853f * v;

	Vector3D w = _dir;
	Vector3D a = std::abs(w.getX()) > 0.9f ? Vector3D{ 0,1,0 } : Vector3D{ 1,0,0 };
	Vector3D uvec = w.cross(a).normalized();
	Vector3D vvec = w.cross(uvec);

	return (uvec * (sinTheta * std::cos(phi)) + vvec * (sinTheta * std::sin(phi)) +
		w * cosTheta).normalized();
}

void PointEmitter::Emit(double dt, std::vector<std::unique_ptr<Particle>>& out) {
	// Cuántas generamos ese fotograma
	float exact = _rate * float(dt);
	int n = int(std::floor(exact));

	// Probabilidad extra
	std::uniform_real_distribution<float> U01(0.f, 1.f);
	if (U01(_R.rng) < (exact - n)) ++n;

	std::uniform_real_distribution<float> Jx(-_posJitter.getX(), _posJitter.getX());
	std::uniform_real_distribution<float> Jy(-_posJitter.getY(), _posJitter.getY());
	std::uniform_real_distribution<float> Jz(-_posJitter.getZ(), _posJitter.getZ());

	for (int i = 0; i < n; ++i) {
		Vector3D p = _pos + Vector3D{ Jx(_R.rng), Jy(_R.rng), Jz(_R.rng) };
		Vector3D dir = RandomDirInCone(_R);
		float speed = std::max<float>(0.f, _speedNormal(_R));
		Vector3D vel{ dir.getX() * speed, dir.getY() * speed, dir.getZ() * speed };

		auto ptr = std::make_unique<Particle>(p, vel, _radius, _color, _life, 0.997f);
		ptr->SetGravity(_gravity);
		out.emplace_back(std::move(ptr));
	}
}

Vector3D BoxEmitter::RandomPos(RNG& R) const {
	std::uniform_real_distribution<float> X(_box.min.getX(), _box.max.getX());
	std::uniform_real_distribution<float> Y(_box.min.getY(), _box.max.getY());
	std::uniform_real_distribution<float> Z(_box.min.getZ(), _box.max.getZ());
	return Vector3D{ X(R.rng), Y(R.rng), Z(R.rng) };
}

Vector3D BoxEmitter::RandomSmallDir(RNG& R) {
	// Direcciones suaves y pequeñas
	Vector3D d = RandomUnit(R); // Aleatoria
	float s = std::max<float>(0.f, _speedNormal(R));
	return d * s;
}

void BoxEmitter::Emit(double dt, std::vector<std::unique_ptr<Particle>>& out) {
	float exact = _rate * float(dt);
	int n = int(std::floor(exact));
	std::uniform_real_distribution<float> U01(0.f, 1.f);
	if (U01(_R.rng) < (exact - n)) ++n;

	for (int i = 0; i < n; ++i) {
		Vector3D p = RandomPos(_R);
		Vector3D v = RandomSmallDir(_R);
		auto ptr = std::make_unique<Particle>(p, v, _radius, _color, _life,
			0.999f);

		ptr->SetGravity(_gravity);
		out.emplace_back(std::move(ptr));
	}
}
