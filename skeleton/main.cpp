#include <ctype.h>

#include <PxPhysicsAPI.h>

#include <vector>

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

#include "Sim/Particles/Particle.h"
#include "Sim/Particles/ProjectileManager.h"

#include "Sim/Particles/ParticleSystem.h"
#include "Sim/Particles/Emitter.h"

#include <iostream>

#include "Sim/Particles/Forces/GravityForce.h"
#include "Sim/Particles/Forces/WindForce.h"
#include "Sim/Particles/Forces/VortexForce.h"
#include "Sim/Particles/Forces/ExplosionForce.h"

std::string display_text = "This is a test";


using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;


PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene      = NULL;
ContactReportCallback gContactReportCallback;

// Shapes
static physx::PxShape* gSphereShape = nullptr;

// Transforms (vida larga para que el renderer los lea cada frame)
static physx::PxTransform gSphereTr{ physx::PxVec3(0.f, 0.f, 0.f) };
static physx::PxTransform gSphereXTr{ physx::PxVec3(10.f, 0.f, 0.f) };
static physx::PxTransform gSphereYTr{ physx::PxVec3(0.f, 10.f, 0.f) };
static physx::PxTransform gSphereZTr{ physx::PxVec3(0.f, 0.f, 10.f) };

// RenderItems (para poder deregistrar en cleanup)
static RenderItem* gSphereItem = nullptr;
static RenderItem* gSphereXItem = nullptr;
static RenderItem* gSphereYItem = nullptr;
static RenderItem* gSphereZItem = nullptr;

Particle* p = nullptr;

static ProjectileManager gProj;

static ParticleSystem gPS;

ExplosionForce* gExplPtr = nullptr;
WindForce* wind = nullptr;

GravityForce* grav1 = nullptr;
GravityForce* grav2 = nullptr;


// Initialize physics engine
void initPhysics(bool interactive)
{
	PX_UNUSED(interactive);

	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// For Solid Rigids +++++++++++++++++++++++++++++++++++++
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);

	// ====== P0: CREACIÓN Y REGISTRO DE GEOMETRÍAS ======

	// Esfera central (radio 1.0)
	gSphereShape = CreateShape(physx::PxSphereGeometry(1.0f), gMaterial);
	gSphereItem = new RenderItem(gSphereShape, &gSphereTr, Vector4(1, 1, 1, 1));
	gSphereXItem = new RenderItem(gSphereShape, &gSphereXTr, Vector4(1, 0, 0, 1)); // rojo
	gSphereYItem = new RenderItem(gSphereShape, &gSphereYTr, Vector4(0, 1, 0, 1)); // verde
	gSphereZItem = new RenderItem(gSphereShape, &gSphereZTr, Vector4(0, 0, 1, 1)); // azul

	// ====== P1: CREACIÓN Y REGISTRO DE LA PARTÍCULA ======

	p = new Particle(Vector3D{ 0,1,0 }, Vector3D{ 5,8,0 }, 0.2f, Vector4(1, 1, 0, 1), 5.0, 0.99f);

	// ====== P2: CREACIÓN Y REGISTRO DEL SISTEMA DE PARTÍCULAS ======
	gPS.SetGravityAll(Vector3D{ 0, -10, 0 });

	// Manguera / fuente: desde un punto, hacia +Z con cono pequeño
	{
		auto pe = std::make_unique<PointEmitter>(
			/*pos*/ Vector3D{ 0, 10, 0 },
			/*dir*/ Vector3D{ 0, 0, 1 },
			/*rate*/ 120.f,				// 120 p/s
			/*speedMean*/ 10.f, /*speedStd*/ 2.f,
			/*life*/ 5.0, /*radius*/ 0.05f,
			/*color*/ Vector4(0.9f, 0.9f, 1, 0.8f)
		);
		pe->SetCone(0.2f);
		pe->SetPositionJitter(Vector3D{ 0.02f, 0.02f, 0.02f });
		gPS.AddEmitter(std::move(pe));
	}

	// Fuente vertical: hacia +Y, salpicada (Gauss)
	{
		auto pe = std::make_unique<PointEmitter>(
			/*pos*/ Vector3D{ -20, 0, 0 },
			/*dir*/ Vector3D{ 0, 1, 0 },
			/*rate*/ 80.f,				// 80 p/s
			/*speedMean*/ 12.f, /*speedStd*/ 3.f,
			/*life*/ 6.0, /*radius*/ 0.05f,
			/*color*/ Vector4(0.6f, 0.8f, 1, 0.9f)
		);
		pe->SetCone(0.35f);
		gPS.AddEmitter(std::move(pe));
	}

	// Niebla: caja con posiciones aleatorias, velocidades pequeñas
	{
		AABB box{ Vector3D{-3,0,-3}, Vector3D{3,1,3} };
		auto be = std::make_unique<BoxEmitter>(box,
			200.f, 0.8f, 0.3f,
			8.0, 0.03f, Vector4(0.8f, 0.8f, 0.9f, 0.3f)
		);
		gPS.AddEmitter(std::move(be));
	}

	gPS.SetBounds(AABB{ Vector3D{-50, -5, -50}, Vector3D{50, 50, 50} });

	// ====== P3: CREACIÓN Y REGISTRO DE LOS GENERADORES DE FUERZA ======

	// Gravedad 1 (normal)
	auto g1 = std::make_unique<GravityForce>(Vector3D{ 0, -9.8, 0 });
	grav1 = g1.get();
	gPS.AddForceGenerator(std::move(g1));

	// Gravedad 2 (más suave) - para comparar con masas distintas
	auto g2 = std::make_unique<GravityForce>(Vector3D{ 0, -3.0, 0 });
	grav2 = g2.get();
	gPS.AddForceGenerator(std::move(g2));

	// Viento en un volumen (k1>0, k2=0 al principio)
	{
		auto w = std::make_unique<WindForce>(Vector3D{ 0, 0, 0 }, 2.0f, 0.0f);
		w->SetVolume(AABB{ Vector3D{-5, 0, -5}, Vector3D{5, 5, 5} });
		wind = w.get();
		gPS.AddForceGenerator(std::move(w));
	}

	// Torbellino centrado en el origen, K = 4
	gPS.AddForceGenerator(std::make_unique<VortexForce>(Vector3D{ 0, 0, 0 }, 4.0f, 1.0f, 0.0f));

	// Explosión (no activa hasta que la dispares)
	auto expl = std::make_unique<ExplosionForce>(Vector3D{ 0, 1, 0 }, 2000.f, 6.f, 1.0f);
	gExplPtr = expl.get(); // guarda puntero para trigger con tecla
	gPS.AddForceGenerator(std::move(expl));

	// viento global muy fuerte (sin volumen) – debería arrastrar todo a +X
	gPS.AddForceGenerator(std::make_unique<WindForce>(Vector3D{ 30, 0, 0 }, /*k1*/ 2.0f, /*k2*/ 0.0f));
}


// Function to configure what happens in each step of physics
// interactive: true if the game is rendering, false if it offline
// t: time passed since last call in milliseconds
void stepPhysics(bool interactive, double t)
{
	PX_UNUSED(interactive);

	// ====== P1: ACTUALIZACIÓN DE LA PARTÍCULA ======
	p->Integrate(t);

	gScene->simulate(t);
	gScene->fetchResults(true);

	gProj.Update(t);

	// ====== P2: ACTUALIZACIÓN DEL SISTEMA DE PARTÍCULAS ======
	gPS.Update(t);
}

// Function to clean data
// Add custom code to the begining of the function
void cleanupPhysics(bool interactive)
{
	// ====== P0: DESREGISTRO Y LIBERACIÓN ======
	if (gSphereItem) { gSphereItem->release(); gSphereItem = nullptr; }
	if (gSphereXItem) { gSphereXItem->release(); gSphereXItem = nullptr; }
	if (gSphereYItem) { gSphereYItem->release(); gSphereYItem = nullptr; }
	if (gSphereZItem) { gSphereZItem->release(); gSphereZItem = nullptr; }

	if (gSphereShape) { gSphereShape->release(); gSphereShape = nullptr; }

	// ====== P1: BORRADO DE LA PARTÍCULA ======
	if (p) { delete p; p = nullptr; }
	gProj.Clear();

	// ====== P2: BORRADO DEL SISTEMA DE PARTÍCULAS ======
	gPS.Clear();

	PX_UNUSED(interactive);

	// Rigid Body ++++++++++++++++++++++++++++++++++++++++++
	gScene->release();
	gDispatcher->release();
	// -----------------------------------------------------
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	
	gFoundation->release();
}

// Function called when a key is pressed
void keyPress(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(camera);

	switch(toupper(key))
	{
	//case 'B': break;
	//case ' ':	break;
	case ' ':
	{
		gProj.Fire();
		break;
	}
	case '1':
	{
		gProj.SetCurrent(ProjectileKind::CannonBall);
		break;
	}
	case '2':
	{
		gProj.SetCurrent(ProjectileKind::TankShell);
		break;
	}
	case '3':
	{
		gProj.SetCurrent(ProjectileKind::Pistol);
		break;
	}
	case 'q':
	{
		gProj.Cycle(-1);
		break;
	}
	case 'e':
	{
		gProj.Cycle(+1);
		break;
	}

	case 'V': {
		wind->SetEnabled(!wind->Enabled());
		std::cout << "Viento " << (wind->Enabled() ? "ON" : "OFF") << "\n"; break;
	}
	case '[': {
		wind->SetK1(wind->GetK1() - 0.5f);
		std::cout << "k1=" << wind->GetK1() << "\n"; break;
	}
	case ']': {
		wind->SetK1(wind->GetK1() + 0.5f);
		std::cout << "k1=" << wind->GetK1() << "\n"; break;
	}
			// ... idem k2, y para vortex: SetK()/K()
	case 'G': { 
		grav1->SetEnabled(!grav1->Enabled());
		std::cout << "grav1=" << grav1->Enabled() << "\n";
		break; 
	}
	case 'H': { 
		grav2->SetEnabled(!grav2->Enabled());
		std::cout << "grav2=" << grav2->Enabled() << "\n";
		break; 
	}
	case 'X': { if (gExplPtr) gExplPtr->Reset(); break; }
	case 'Z': { if (gExplPtr) gExplPtr->Stop();  break; }


	default:
		break;
	}
}

void onCollision(physx::PxActor* actor1, physx::PxActor* actor2)
{
	PX_UNUSED(actor1);
	PX_UNUSED(actor2);
}


int main(int, const char*const*)
{
#ifndef OFFLINE_EXECUTION 
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}