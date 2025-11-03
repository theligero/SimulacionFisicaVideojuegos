#ifndef EMITTER_H_
#define EMITTER_H_

#pragma once

#include <memory>
#include <vector>
#include "PxPhysicsAPI.h"
#include "../Math/Vector3D.h"
#include "Distributions.h"
#include "../../core.hpp"

class Particle;

struct AABB {
	Vector3D min, max;
};

class IEmitter {
public:
	virtual ~IEmitter() = default;

	// Genera n partículas y se las añade al vector
	virtual void Emit(double dt, std::vector<std::unique_ptr<Particle>>& out) = 0;
};

// Emisor puntual: posición fija (con jitter), velocidad según distribución
class PointEmitter : public IEmitter {
private:
	Vector3D _pos;
	Vector3D _dir;
	Vector3D _posJitter{ 0,0,0 };
	float _halfAngle = 0.0f; // 0 = rayo; >0 = cono

	float _rate; // particulas / seg
	Normal _speedNormal;
	double _life;
	float _radius;
	Vector4 _color;

	Vector3D _gravity{ 0, -10, 0 };
	RNG _R;

	Vector3D RandomDirInCone(RNG& R) const;
public:
	PointEmitter(const Vector3D& pos, const Vector3D& dirBase, float rate, 
		float speedMean, float speedStd, double lifeSec, float radius, const Vector4& color) :
		_pos(pos), _dir(dirBase.normalized()), _rate(rate), _speedNormal(speedMean, speedStd),
		_life(lifeSec), _radius(radius), _color(color) {}

	void SetPositionJitter(const Vector3D& j) { _posJitter = j; }
	void SetCone(float halfAngleRad) { _halfAngle = halfAngleRad; }

	void SetGravity(const Vector3D& g) { _gravity = g; }

	void Emit(double dt, std::vector<std::unique_ptr<Particle>>& out) override;
};

// Emisor volumétrico (niebla): posición uniforme en caja, velocidad pequeña gaussiana
class BoxEmitter : public IEmitter {
private:
	Vector3D RandomPos(RNG& R) const;
	Vector3D RandomSmallDir(RNG& R);

	AABB _box;
	float _rate;
	Normal _speedNormal;
	double _life;
	float _radius;
	Vector4 _color;

	Vector3D _gravity{ 0, -10, 0 };
	RNG _R;

public:
	BoxEmitter(const AABB& box, float rate, float speedMean, float speedStd,
		double lifeSec, float radius, const Vector4& color) :
		_box(box), _rate(rate), _speedNormal(speedMean, speedStd), _life(lifeSec),
		_radius(radius), _color(color) {}

	void SetGravity(const Vector3D& g) { _gravity = g; }
	void Emit(double dt, std::vector<std::unique_ptr<Particle>>& out) override;
};
#endif // EMITTER_H_