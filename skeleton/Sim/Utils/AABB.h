#ifndef AABB_H_
#define AABB_H_

#pragma once

#include "../Math/Vector3D.h"

struct AABB {
	Vector3D min, max;
};

#endif // AABB_H_