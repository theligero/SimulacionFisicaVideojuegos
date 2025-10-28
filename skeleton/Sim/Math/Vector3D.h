#ifndef VECTOR_3D_H_
#define VECTOR_3D_H_

#pragma once

class Vector3D {
private:
	float _x, _y, _z;

public:
	Vector3D() : _x(0), _y(0), _z(0) {}
	Vector3D(float x, float y, float z) : _x(x), _y(y), _z(z) {}

	float length() const;
	Vector3D normalized() const;

	float dot(const Vector3D& o) const;
	Vector3D cross(const Vector3D& o) const;
	Vector3D operator*(float s) const;
	Vector3D operator+(const Vector3D& o) const;
	Vector3D operator-(const Vector3D& o) const;

	float getX() const;
	float getY() const;
	float getZ() const;
};

#endif // VECTOR_3D_H_