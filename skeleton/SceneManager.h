#ifndef SCENE_MANAGER_H_
#define SCENE_MANAGER_H_

#include <list>
#include "Particle.h"
#include "Render/Camera.h"

class SceneManager
{
private:
	Camera* cam;
	std::list<Particle*> projectiles;
	int num_projectiles;
public:
	SceneManager(Camera* c, int p);
	~SceneManager();

	void update(double t);
	void addProjectile();
};

#endif SCENE_MANAGER_H_