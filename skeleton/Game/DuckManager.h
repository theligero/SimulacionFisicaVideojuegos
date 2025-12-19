#ifndef DUCK_MANAGER_H_
#define DUCK_MANAGER_H_

#pragma once

#include <vector>

namespace physx {
	class PxRigidDynamic;
	class PxPhysics;
	class PxScene;
}

class RenderItem;

struct Duck {
	physx::PxRigidDynamic* body;
	RenderItem* renderItem;
	double life;
	bool isDead;
};

class DuckManager {
public:
	DuckManager(physx::PxPhysics* physics, physx::PxScene* scene);
	~DuckManager();

	void Update(double dt, double waterLevel);
	void SpawnDuck();

	const std::vector<Duck*>& GetDucks() const;

private:
	std::vector<Duck*> _ducks;
	physx::PxPhysics* _physics;
	physx::PxScene* _scene;
	double _spawnTimer;
};

#endif // DUCK_MANAGER_H_