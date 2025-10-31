#include <ctype.h>

#include <PxPhysicsAPI.h>

#include <vector>

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

#include "Sim/Particles/Particle.h"
#include "Sim/Particles/ProjectileManager.h"

#include <iostream>

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