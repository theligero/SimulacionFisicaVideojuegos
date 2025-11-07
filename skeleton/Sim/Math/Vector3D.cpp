#include "Vector3D.h"

#include <cmath>

float Vector3D::length() const 
{
	return std::sqrt(_x * _x + _y * _y + _z * _z);
}

Vector3D Vector3D::normalized() const 
{
	float L = length();
	return (L > 0.0f) ? Vector3D{ _x / L, _y / L, _z / L } : Vector3D{};
}

float Vector3D::dot(const Vector3D& o) const
{
	return _x * o._x + _y * o._y + _z * o._z;
}

Vector3D Vector3D::cross(const Vector3D& o) const
{
	return Vector3D{ _y*o._z - _z*o._y, _z*o._x - _x*o._z, _x*o._y - _y*o._x };
}

Vector3D Vector3D::operator*(float s) const 
{
	return Vector3D{ _x * s, _y * s, _z * s };
}

Vector3D Vector3D::operator+(const Vector3D& o) const
{
	return Vector3D{ _x + o._x, _y + o._y, _z + o._z };
}

Vector3D Vector3D::operator-(const Vector3D& o) const
{
	return Vector3D{ _x - o._x, _y - o._y, _z - o._z };
}

Vector3D Vector3D::operator+=(const Vector3D& o)
{
	_x += o._x; _y += o._y; _z += o._z;
	return *this;
}

Vector3D Vector3D::operator-=(const Vector3D& o)
{
	_x -= o._x; _y -= o._y; _z -= o._z;
	return *this;
}

Vector3D Vector3D::operator*=(float s)
{
	_x *= s; _y *= s; _z *= s;
	return *this;
}

float Vector3D::getX() const
{
	return _x;
}

float Vector3D::getY() const
{
	return _y;
}

float Vector3D::getZ() const
{
	return _z;
}
