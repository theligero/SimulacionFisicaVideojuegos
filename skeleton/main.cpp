#include <PxPhysics.h>

#include "Sim/Particles/Emitter.h"
#include "Sim/Particles/ProjectileManager.h"
#include "Sim/Particles/ParticleSystem.h"
#include "Sim/Particles/Forces/WindForce.h"
#include "Sim/Particles/Forces/GravityForce.h"

#include "Game/GameManager.h"
#include "Game/DuckManager.h"
#include "Game/HUD.h"

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

#include <iostream>
#include <memory>
#include <cctype>

// -------------------------------------------------------------------------
// VARIABLES GLOBALES
// -------------------------------------------------------------------------
physx::PxDefaultAllocator			gAllocator;
physx::PxDefaultErrorCallback		gErrorCallback;

physx::PxFoundation*				gFoundation		= NULL;
physx::PxPhysics*					gPhysics		= NULL;

physx::PxMaterial*					gMaterial		= NULL;

physx::PxPvd*						gPvd			= NULL;

physx::PxDefaultCpuDispatcher*		gDispatcher		= NULL;
physx::PxScene*						gScene			= NULL;
ContactReportCallback				gContactCB;

// Instancias globales
GameManager* gGame = nullptr;
DuckManager* gDucks = nullptr;
ProjectileManager* gProj = nullptr;

// Elementos visuales extra
RenderItem* gWaterRenderItem = nullptr;

// Generadores de fuerza
ParticleSystem* gParticleSys = nullptr;
WindForce* gWindForce = nullptr;
GravityForce* gGravityForce = nullptr;

physx::PxTransform gWaterPose;


// -------------------------------------------------------------------------
// FUNCIONES AUXILIARES
// -------------------------------------------------------------------------

// Comprobación de colisiones: balas (partículas) vs patos (sólidos)
void checkCollisions() {
	if (!gDucks || !gProj || !gGame) return;

	// Obtenemos las listas
	const auto& ducks = gDucks->GetDucks();
	const auto& particles = gProj->GetParticles();

	for (auto& duck : ducks) {
		if (duck->isDead) continue;

		physx::PxTransform t = duck->body->getGlobalPose();
		Vector3D duckPos(t.p.x, t.p.y, t.p.z);
		float duckRadius = 1.5f;

		for (const auto& p : particles) {
			if (!p || !p->IsAlive()) continue;

			Vector3D diff = p->GetPosition() - duckPos;
			float distSq = diff.getX() * diff.getX() + 
				diff.getY() * diff.getY() + diff.getZ() * diff.getZ();

			if (distSq < (duckRadius * duckRadius)) {
				// IMPACTO
				duck->isDead = true;
				gGame->OnDuckHit();

				// Feedback físico: empujar elpato con la velocidad de la bala
				Vector3D v = p->GetVelocity();
				duck->body->addForce(physx::PxVec3(v.getX(), v.getY(), v.getZ()) * 10.0f);
				duck->body->setAngularVelocity(physx::PxVec3(5, 5, 5)); // girar al morir

				// Feedback consola
				std::cout << "Pato cazado! Score: " << gGame->GetScore() << std::endl;

				break; // solo un pato muere por frame
			}
		}
	}
}

// -------------------------------------------------------------------------
// CORE
// -------------------------------------------------------------------------

void initPhysics(bool interactive) {
	PX_UNUSED(interactive);

	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, 
		physx::PxTolerancesScale(), true, gPvd);

	// Material por defecto
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Para sólido rígidos
	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactCB;
	gScene = gPhysics->createScene(sceneDesc);

	// --- ESCENARIO ---
	// Agua visual (plano grande azul transparente)
	physx::PxShape* waterShape = 
		gPhysics->createShape(physx::PxBoxGeometry(100.0f, 0.1f, 50.0f), *gMaterial);
	gWaterPose.p = physx::PxVec3(0, -0.2f, 0);
	gWaterRenderItem = new RenderItem(waterShape, &gWaterPose, Vector4(0.0f, 0.2f, 1.0f, 0.6f));
	waterShape->release();


	// --- MANAGERS ---
	gGame = new GameManager();
	gDucks = new DuckManager(gPhysics, gScene);
	gProj = new ProjectileManager();

	// Sistema de partículas
	gParticleSys = new ParticleSystem();

	// Límites para limpiar por espacio
	gParticleSys->SetBounds(AABB{ Vector3D{-200, -50,-200}, Vector3D{200, 200, 200} });

	// Dos emisores distintos (VISIBLES)
	auto mist = std::make_unique<BoxEmitter>(
		AABB{ Vector3D{-100, -0.1f, -40}, Vector3D{100, 0.2, 40} }, // alrededor del agua
		500.0f, // rate
		0.8f, 0.4f, // speed mean/std
		2.0, // life
		0.125f, // radius
		Vector4(0.05f, 0.05f, 1.0f, 1.0f) // color
	);
	mist->SetGravity(Vector3D{ 0, 0.0f, 0 });
	gParticleSys->AddEmitter(std::move(mist));

	auto sparks1 = std::make_unique<PointEmitter>(
		Vector3D{-100, 20, -25 },		// un punto visible en escena
		Vector3D{1, 0.2f, 0},		// dirección base
		250.0f, 12.0f, 4.0f,		// rate, speed mean/std
		1.2, 0.125f,
		Vector4(0.05f, 0.05f, 1.0f, 1.0f)
	);
	sparks1->SetCone(0.6f);
	sparks1->SetPositionJitter(Vector3D{ 0.5f, 0.2f, 0.5f });
	gParticleSys->AddEmitter(std::move(sparks1));

	auto sparks2 = std::make_unique<PointEmitter>(
		Vector3D{ -100, 20, 25 },		// un punto visible en escena
		Vector3D{ 1, 0.2f, 0 },		// dirección base
		250.0f, 12.0f, 4.0f,		// rate, speed mean/std
		1.2, 0.125f,
		Vector4(0.05f, 0.05f, 1.0f, 1.0f)
	);
	sparks2->SetCone(0.6f);
	sparks2->SetPositionJitter(Vector3D{ 0.5f, 0.2f, 0.5f });
	gParticleSys->AddEmitter(std::move(sparks2));

	// Gravedad (fuerza constante)
	auto gravity = std::make_unique<GravityForce>(Vector3D(0, -9.8f, 0));
	gGravityForce = gravity.get();
	gParticleSys->AddForceGenerator(std::move(gravity));

	// Viento (fuerza variable)
	auto wind = std::make_unique<WindForce>(Vector3D(10.0f, 0.0f, 0.0f), 1.0f, 0.1f);
	gWindForce = wind.get();
	gParticleSys->AddForceGenerator(std::move(wind));
}

void stepPhysics(bool interactive, double dt) {
	PX_UNUSED(interactive);

	// Simulación física
	gScene->simulate(dt);
	gScene->fetchResults(true);

	// Lógica de juego
	if (gGame) {
		gGame->Update(dt);

		if (gGame->GetState() == GameState::PLAYING) {
			if (gParticleSys) gParticleSys->Update(dt);
			// Actualizar managers específicos
			if (gDucks) {
				gDucks->Update(dt, gWaterPose.p.y);

				if (gWindForce) {
					for (auto& duck : gDucks->GetDucks()) {
						if (!duck->isDead && duck->body) {
							gWindForce->Apply(duck->body, dt);
						}
					}
				}
			}

			if (gProj) gProj->Update(dt);

			// Colisiones
			checkCollisions();
		}
	}
}

void cleanupPhysics(bool interactive) {
	PX_UNUSED(interactive);

	// Limpiar managers
	delete gGame;
	delete gDucks; // su destructor limpia los patos
	if (gProj) { gProj->Clear(); delete gProj; }

	// Limpiar escenario
	if (gWaterRenderItem) gWaterRenderItem->release();

	// Limpiar generador de partículas
	if (gParticleSys) { delete gParticleSys; gParticleSys = nullptr; }

	// Limpiar PhysX
	gScene->release();
	gDispatcher->release();
	gPhysics->release();

	if (gPvd) {
		physx::PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();
	}

	gFoundation->release();
}

void onCollision(physx::PxActor* actor1, physx::PxActor* actor2) {
	PX_UNUSED(actor1);
	PX_UNUSED(actor2);
}

void keyPress(unsigned char key, const physx::PxTransform& camera) {
	if (!gGame) return;

	switch (toupper(key)) {
	case ' ':
		if (gGame->GetState() == GameState::MENU) {
			gGame->StartGame();
		}
		else if (gGame->GetState() == GameState::PLAYING) {
			gProj->Fire();
			gGame->OnShotFired();
		}
		break;
	// Reiniciar en Game Over
	case 'R':
		if (gGame->GetState() == GameState::GAME_OVER) {
			gGame->StartGame(); // reiniciar variables
		}
		break;
	}
}

int main(int, const char* const*) {
#ifndef OFFLINE_EXECUTION
	extern void renderLoop();
	renderLoop();
#else
	static const physx::PxU32 frameCount = 100;
	initPhysics(false);
	for (physx::PxU32 i = 0; < frameCount; i++) {
		stepPhysics(false);
	}
	cleanupPhysics(false);
#endif

	return 0;
}