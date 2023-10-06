#include "SceneManager.h"

SceneManager::SceneManager(Camera* c, int p)
{
	cam = c;
	num_projectiles = p;
}

SceneManager::~SceneManager()
{
	for (auto& e : projectiles) {
		delete e;
	}
}

void SceneManager::update(double t)
{
	for (auto& e : projectiles) {
		e->integrate(t);
	}
}

void SceneManager::addProjectile(typeOfParticle type)
{
	if (projectiles.size() + 1 > num_projectiles) {
		delete projectiles.front();
		projectiles.erase(projectiles.begin());
	}

	Particle* p = nullptr;

	switch (type) {
		case BULLET:
			p = new Particle(cam->getTransform(), { cam->getDir().x * 35.0f, cam->getDir().y, cam->getDir().z * 40.0f },
				{ 0.0f, -5.8f, 0.0f }, { 0, 0, 0.5, 1.0 }, damping, 50.0f);
			break;
		case CANNON:
			p = new Particle(cam->getTransform(), { cam->getDir().x * 55.0f, cam->getDir().y, cam->getDir().z * 60.0f },
				{ 0.0f, -3.5f, 0.0f }, { 0.0, 0.0, 0.0, 1.0 }, damping, 200.0f);
			break;
		case MISSILE:
			p = new Particle(cam->getTransform(), { cam->getDir().x * 125.0f, cam->getDir().y, cam->getDir().z * 100.0f },
				{ 0.0f, -1.5f, 0.0f }, { 1.0, 0.0, 1.0, 1.0 }, damping, 100.0f);
			break;
	}

	projectiles.push_back(p);
}
