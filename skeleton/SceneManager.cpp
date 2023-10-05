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

void SceneManager::addProjectile()
{
	Particle *p = new Particle(cam->getTransform(), { cam->getDir().x * 35.0f, cam->getDir().y, cam->getDir().z * 40.0f }, 
		{ 0.0f, -9.8f, 0.0f }, 0.9995f, 100.0f);
	projectiles.push_back(p);
}
