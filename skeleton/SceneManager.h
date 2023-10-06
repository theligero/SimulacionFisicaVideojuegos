#ifndef SCENE_MANAGER_H_
#define SCENE_MANAGER_H_

#include <list>
#include "Particle.h"
#include "Render/Camera.h"

const float damping = 0.9995f;

enum typeOfParticle { BULLET, CANNON, MISSILE };

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
	void addProjectile(typeOfParticle type);
};

#endif SCENE_MANAGER_H_