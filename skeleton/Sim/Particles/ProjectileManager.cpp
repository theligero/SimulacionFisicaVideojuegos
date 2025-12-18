#include "ProjectileManager.h"

ProjectileManager::ProjectileManager()
{
	_cannon = ProjectileDesc{ 5.0f, 250.f, 20.f, -2.0f, 0.20f, 
		Vector4(0.9f, 0.9f, 0.2f, 1), 6.0, 0.995f };
	_tank = ProjectileDesc{ 10.0f, 1800.f, 25.f, -0.8f, 0.22f,
		Vector4(0.95f, 0.35f, 0.35f, 1), 6.0, 0.997f };
	_pistol = ProjectileDesc{ 0.01f, 330.f, 15.f, -0.5f, 0.12f,
		Vector4(0.2f, 0.7f, 1.0f, 1), 5.0f, 0.998f };
}

void ProjectileManager::SetPreset(ProjectileKind k, const ProjectileDesc& d) {
	switch (k) {
	case ProjectileKind::CannonBall:
		_cannon = d;
		break;
	case ProjectileKind::TankShell:
		_tank = d;
		break;
	case ProjectileKind::Pistol:
		_pistol = d;
		break;
	}
}

const ProjectileDesc& ProjectileManager::GetPreset(ProjectileKind k) const {
	return Pick(k);
}

const ProjectileDesc& ProjectileManager::Pick(ProjectileKind k) const {
	switch (k) {
	case ProjectileKind::CannonBall:
		return _cannon;
	case ProjectileKind::TankShell:
		return _tank;
	case ProjectileKind::Pistol:
		return _pistol;
	}

	return _cannon;
}

void ProjectileManager::SetCurrent(ProjectileKind k)
{
	_current = k;
}

ProjectileKind ProjectileManager::Current() const
{
	return _current;
}

void ProjectileManager::Cycle(int dir) {
	int i = static_cast<int>(_current);
	int n = 3;
	i = (i + (dir >= 0 ? 1:-1) + n) % n;
	_current = static_cast<ProjectileKind>(i);
}

void ProjectileManager::Fire() {
	auto* cam = GetCamera();
	const physx::PxVec3 eye = cam->getEye();
	const physx::PxVec3 dir = cam->getDir().getNormalized();

	const auto& d = Pick(_current);

	Vector3D pos{ eye.x, eye.y, eye.z };
	// Empuja la bala un poco hacia delante para que no nazca dentro de la cámara (near plane)
	pos = pos + Vector3D{ dir.x, dir.y, dir.z } * (d._radius * 2.0f + 1.0f);
	Vector3D vel{ dir.x * d._vSim, dir.y * d._vSim, dir.z * d._vSim };

	float m_sim = compute_m_sim(d._mReal, d._vReal, d._vSim);

	auto p = std::make_unique<Particle>(pos, vel, d._radius, d._color, d._life, d._damping);
	p->SetMass(m_sim);
	p->SetGravity(Vector3D{ 0.f, d._gSim, 0.f });

	_active.emplace_back(std::move(p));
}

void ProjectileManager::Update(double dt) {
	for (auto& p : _active) if (p) p->Integrate(dt);

	_active.erase(
		std::remove_if(_active.begin(), _active.end(),
			[](const std::unique_ptr<Particle>& p) {
				return !p || !p->IsAlive();
			}),
		_active.end());
}

void ProjectileManager::Clear() {
	_active.clear();
}

size_t ProjectileManager::Size() const
{
	return _active.size();
}

const std::vector<std::unique_ptr<Particle>>& ProjectileManager::GetParticles() const
{
	return _active;
}
