#include "DuckManager.h"

#include "../RenderUtils.hpp"
#include <PxPhysicsAPI.h>
#include <iostream>

extern physx::PxMaterial* gMaterial;

bool GetVolumeAndHalfHeight(physx::PxShape* shape, float& outVolume, float& outHalfH)
{
	using namespace physx;

	switch (shape->getGeometryType())
	{
	case PxGeometryType::eBOX: {
		PxBoxGeometry g;
		if (!shape->getBoxGeometry(g)) return false;
		outVolume = 8.0f * g.halfExtents.x * g.halfExtents.y * g.halfExtents.z;
		outHalfH = g.halfExtents.y;
		return true;
	}
	case PxGeometryType::eSPHERE: {
		PxSphereGeometry g;
		if (!shape->getSphereGeometry(g)) return false;
		outVolume = (4.0f / 3.0f) * PxPi * g.radius * g.radius * g.radius;
		outHalfH = g.radius;
		return true;
	}
	case PxGeometryType::eCAPSULE: {
		PxCapsuleGeometry g;
		if (!shape->getCapsuleGeometry(g)) return false;
		const float r = g.radius;
		const float hh = g.halfHeight;
		outVolume = PxPi * r * r * (2.0f * hh) + (4.0f / 3.0f) * PxPi * r * r * r;
		outHalfH = hh + r;
		return true;
	}
	default:
		return false;
	}
}


DuckManager::DuckManager(physx::PxPhysics* physics, physx::PxScene* scene) :
	_physics(physics), _scene(scene), _spawnTimer(0.0)
{

}

DuckManager::~DuckManager()
{
	for (auto& duck : _ducks) {
		if (duck->renderItem) duck->renderItem->release();
		if (duck->body) duck->body->release();
		delete duck;
	}

	_ducks.clear();
}

void DuckManager::Update(double dt, double waterLevel)
{
	// Spawning automático
	_spawnTimer += dt;
	if (_spawnTimer > 2.0) { // un pato cada 2s
		SpawnDuck();
		_spawnTimer = 0.0;
	}

	// Actualizar patos existentes
	auto it = _ducks.begin();
	while (it != _ducks.end()) {
		Duck* d = *it;
		d->life -= dt;

		// --- FLOTACIÓN (FÍSICA) ---
		physx::PxTransform t = d->body->getGlobalPose();

		physx::PxShape* shape = nullptr;
		d->body->getShapes(&shape, 1);

		float volume = 0.0f;
		float halfH = 0.0f;

		if (shape && GetVolumeAndHalfHeight(shape, volume, halfH) && halfH > 0.0001f && volume > 0.0001f)
		{
			const float bottom = t.p.y - halfH;
			if (waterLevel > bottom) // si el agua está por encima del “suelo” del pato
			{
				const float submergedHeight = physx::PxClamp<float>(waterLevel - bottom, 0.0f, 2.0f * halfH);
				const float frac = submergedHeight / (2.0f * halfH);       // 0..1 aprox
				const float volSub = volume * frac;

				const float rhoFluid = 1000.0f; // “agua” (ajustable)
				const float g = 9.8f;

				const float Fb = rhoFluid * volSub * g; // empuje
				physx::PxVec3 buoyancy(0.0f, Fb, 0.0f);

				// Drag solo cuando está en agua
				physx::PxVec3 vel = d->body->getLinearVelocity();
				const float linearDrag = 6.0f; // sube/baja para que no “surfeen”
				physx::PxVec3 drag = -vel * linearDrag * frac;

				d->body->addForce(buoyancy + drag);
				d->body->wakeUp();
			}
		}

		// --- LIMPIEZA --
		// Si mueren (tiempo, caída al vacío o disparo)
		if (d->life <= 0 || t.p.y < -50.0f || (d->isDead && t.p.y < -10.0f)) {
			d->renderItem->release();
			d->body->release();
			delete d;
			it = _ducks.erase(it);
		}
		else {
			// Si le han disparado, cambiar color a rojo (feedback visual)
			if (d->isDead) {
				d->renderItem->color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			++it;
		}
	}
}

void DuckManager::SpawnDuck()
{
	if (!_physics || !_scene) return;

	// Generar posición aleatoria (ej: lado izquierda, altura variable)
	float x = -40.0f;
	float y = 10.0f + (rand() % 10);
	float z = (rand() % 40) - 20.0f;

	// Aleatoriedad en dimensiones
	float w = 3.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.7f));
	float h = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.4f));
	float d = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.6f));

	physx::PxTransform t(physx::PxVec3(x, y, z));
	physx::PxRigidDynamic* body = _physics->createRigidDynamic(t);

	// Usamos gMaterial si está disponible, si no, creamos uno temporal
	physx::PxMaterial* mat = gMaterial;
	if (!mat) mat = _physics->createMaterial(0.5f, 0.5f, 0.6f);

	// Usamos las dimensiones aleatorias
	physx::PxShape* shape = nullptr;

	int kind = rand() % 3; // 0 box, 1 sphere, 2 capsule
	if (kind == 0) {
		shape = _physics->createShape(physx::PxBoxGeometry(w, h, d), *mat);
	}
	else if (kind == 1) {
		float r = 0.5f * (w + d) * 0.5f;
		shape = _physics->createShape(physx::PxSphereGeometry(r), *mat);
	}
	else {
		float r = 0.35f * (w + d) * 0.5f;
		float halfH = std::max<float>(0.2f, h);
		shape = _physics->createShape(physx::PxCapsuleGeometry(r, halfH), *mat);
	}

	body->attachShape(*shape);

	// Densidad aleatoria => masa e inercia distintas
	float density = 300.0f + (rand() % 600); // 300..899
	physx::PxRigidBodyExt::updateMassAndInertia(*body, density);

	// Velocidad inicial hacia la derecha
	body->setLinearVelocity(physx::PxVec3(15.0f + (rand() % 10), 5.0f, 0));
	body->setAngularDamping(0.5f);

	_scene->addActor(*body);

	// Render Item (visual)
	Vector4 color(1.0f, 0.8f, 0.2f, 1.0f); // Amarillo
	RenderItem* item = new RenderItem(shape, body, color);

	// Crear struct
	Duck* newDuck = new Duck();
	newDuck->body = body;
	newDuck->renderItem = item;
	newDuck->life = 20.0; // 20 segundos antes de desaparecer si nadie le da
	newDuck->isDead = false;

	_ducks.push_back(newDuck);

	shape->release(); // el body ya tiene la referencia
}

const std::vector<Duck*>& DuckManager::GetDucks() const
{
	return _ducks;
}
