#ifndef PROJECTILE_MANAGER_H_
#define PROJECTILE_MANAGER_H_

#include <vector>
#include <memory>
#include "../Math/Vector3D.h"
#include "../../RenderUtils.hpp"
#include "Particle.h"

enum class ProjectileKind { CannonBall, TankShell, Pistol };

struct ProjectileDesc {
	float _mReal;
	float _vReal;
	float _vSim;
	float _gSim;
	float _radius;
	Vector4 _color;
	double _life;
	float _damping;
};

inline float compute_m_sim(float m_real, float v_real, float v_sim) {
	const float r = v_real / v_sim; return m_real * r * r;
}

class ProjectileManager {
private:
	const ProjectileDesc& Pick(ProjectileKind k) const;

	std::vector<std::unique_ptr<Particle>> _active;
	ProjectileKind _current{ ProjectileKind::CannonBall };
	ProjectileDesc _cannon, _tank, _pistol;

public:
	ProjectileManager();

	void SetPreset(ProjectileKind k, const ProjectileDesc& d);
	const ProjectileDesc& GetPreset(ProjectileKind k) const;

	void SetCurrent(ProjectileKind k);
	ProjectileKind Current() const;
	void Cycle(int dir); // -1 o +1

	void Fire();
	void Update(double dt);
	void Clear();

	size_t Size() const;
};

#endif // PROJECTILE_MANAGER_H_