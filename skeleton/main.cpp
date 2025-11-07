// main.cpp – Prototipo simple “Tiro al plato”
// Controles: ESPACIO (disparar), 1/2/3 (tipo), V (viento ON/OFF), [ ] (k1 viento),
//            G (gravedad ON/OFF), C (añadir otro lanzador de platos)

#include <ctype.h>
#include <PxPhysicsAPI.h>

#include <vector>
#include <memory>
#include <iostream>

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

// Mi código
#include "Sim/Particles/ParticleSystem.h"
#include "Sim/Particles/Emitter.h"
#include "Sim/Particles/ProjectileManager.h"

#include "Sim/Particles/Forces/GravityForce.h"
#include "Sim/Particles/Forces/WindForce.h"
#include "Sim/Particles/Forces/VortexForce.h"
#include "Sim/Particles/Forces/ExplosionForce.h"

using namespace physx;

// ===== PhysX boilerplate =====
PxDefaultAllocator         gAllocator;
PxDefaultErrorCallback     gErrorCallback;
PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxMaterial* gMaterial = nullptr;
PxPvd* gPvd = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;
ContactReportCallback      gContactCB;

// ===== Juego / Sim =====
static ProjectileManager   gProj;
static ParticleSystem      gPS;

// Fuerzas con toggles
static GravityForce* gGravity = nullptr; // gravedad global
static WindForce* gWind = nullptr; // viento global

// Para añadir “lanzadores de platos” con la tecla C
static std::vector<PointEmitter*> gClayLaunchers;

std::string display_text = "This is a test";

// ========== Helpers ==========
static PointEmitter* makeClayLauncher(
    const Vector3D& pos,
    const Vector3D& dir,
    float rate = 1.5f,           // ~1-2 platos por segundo
    float speedMean = 25.f,
    float speedStd = 3.f,
    double life = 8.0,       // que duren un poco
    float radius = 0.25f)     // que se vean más grandecitos
{
    auto pe = std::make_unique<PointEmitter>(
        pos, dir, rate, speedMean, speedStd, life, radius,
        Vector4(1.0f, 0.6f, 0.2f, 1.0f)   // color naranja “plato”
    );
    pe->SetCone(0.15f);                 // leve dispersión
    pe->SetPositionJitter(Vector3D{ 0.05f, 0.02f, 0.05f });

    PointEmitter* raw = pe.get();
    gPS.AddEmitter(std::move(pe));
    return raw;
}

// ========== Init ==========
void initPhysics(bool interactive)
{
    PX_UNUSED(interactive);

    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxSceneDesc desc(gPhysics->getTolerancesScale());
    desc.gravity = PxVec3(0.f, -9.8f, 0.f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    desc.cpuDispatcher = gDispatcher;
    desc.filterShader = contactReportFilterShader;
    desc.simulationEventCallback = &gContactCB;
    gScene = gPhysics->createScene(desc);

    // ===== Mundo de partículas =====
    // Límites amplios
    gPS.SetBounds(AABB{ Vector3D{-80, -10, -80}, Vector3D{ 80, 60, 80 } });

    // Gravedad global (toggle con G)
    auto g = std::make_unique<GravityForce>(Vector3D{ 0, -9.8f, 0 });
    gGravity = g.get();
    gPS.AddForceGenerator(std::move(g));

    // Viento global suave hacia +X (toggle con V, ajustar con [ ])
    auto w = std::make_unique<WindForce>(Vector3D{ 15.f, 0.f, 0.f }, /*k1*/1.0f, /*k2*/0.2f);
    gWind = w.get();
    gPS.AddForceGenerator(std::move(w));

    // (Opcional) pequeña “brisa” local en un volumen (comentado por simplicidad)
    // {
    //     auto wvol = std::make_unique<WindForce>(Vector3D{ 0.f, 0.f, 12.f }, 1.0f, 0.0f);
    //     wvol->SetVolume(AABB{ Vector3D{-10, 0, -2}, Vector3D{10, 6, 10} });
    //     gPS.AddForceGenerator(std::move(wvol));
    // }

    // Un lanzador de platos por defecto:
    // Colócalo a la izquierda del “campo”, lanzando ligeramente hacia arriba y adelante (+X, +Y, +Z)
    {
        Vector3D pos{ -30.f, 2.f, -15.f };
        Vector3D dir{ 1.f, 0.25f, 0.25f }; // un poco hacia arriba y al frente
        gClayLaunchers.push_back(makeClayLauncher(pos, dir));
    }

    // Nieblilla/partículas ambiente para dar contexto (muy sutil)
    {
        AABB box{ Vector3D{-5, 0, -5}, Vector3D{5, 2, 5} };
        auto be = std::make_unique<BoxEmitter>(box,
            /*rate*/ 80.f, /*speedMean*/ 0.6f, /*speedStd*/ 0.25f,
            /*life*/ 6.0, /*radius*/ 0.035f, Vector4(0.85f, 0.9f, 1.f, 0.25f));
        gPS.AddEmitter(std::move(be));
    }

    // (Opcional) pequeño vórtice para gracia visual (OFF por simplicidad)
    // gPS.AddForceGenerator(std::make_unique<VortexForce>(Vector3D{0,0,0}, 3.0f, 1.0f, 0.0f));
}

// ========== Step ==========
void stepPhysics(bool interactive, double dt)
{
    PX_UNUSED(interactive);

    gScene->simulate(dt);
    gScene->fetchResults(true);

    gProj.Update(dt);
    gPS.Update(dt);
}

// ========== Cleanup ==========
void cleanupPhysics(bool interactive)
{
    PX_UNUSED(interactive);

    gProj.Clear();
    gPS.Clear();

    gScene->release();
    gDispatcher->release();

    gPhysics->release();
    PxPvdTransport* transport = gPvd->getTransport();
    gPvd->release();
    transport->release();

    gFoundation->release();
}

// ========== Input ==========
void keyPress(unsigned char key, const PxTransform& camera)
{
    PX_UNUSED(camera);

    switch (toupper(key))
    {
    case ' ':
        gProj.Fire();
        break;

    case '1': gProj.SetCurrent(ProjectileKind::CannonBall); break;
    case '2': gProj.SetCurrent(ProjectileKind::TankShell);  break;
    case '3': gProj.SetCurrent(ProjectileKind::Pistol);     break;

    case 'V':
        if (gWind) {
            gWind->SetEnabled(!gWind->Enabled());
            std::cout << "[Wind] " << (gWind->Enabled() ? "ON" : "OFF") << "\n";
        }
        break;

    case '[':
        if (gWind) { gWind->SetK1(gWind->GetK1() - 0.5f); std::cout << "k1=" << gWind->GetK1() << "\n"; }
        break;

    case ']':
        if (gWind) { gWind->SetK1(gWind->GetK1() + 0.5f); std::cout << "k1=" << gWind->GetK1() << "\n"; }
        break;

    case 'G':
        if (gGravity) {
            gGravity->SetEnabled(!gGravity->Enabled());
            std::cout << "[Gravity] " << (gGravity->Enabled() ? "ON" : "OFF") << "\n";
        }
        break;

    case 'C':
    {
        // Añade otro lanzador (posición un poco distinta para variedad)
        float x = -30.f, y = 2.f, z = -15.f + (float)(rand() % 10 - 5);
        Vector3D pos{ x, y, z };
        Vector3D dir{ 1.f, 0.28f, 0.18f };
        auto pe = makeClayLauncher(pos, dir, /*rate*/1.0f);
        gClayLaunchers.push_back(pe);
        std::cout << "[Clay] Nuevo lanzador añadido. Total=" << gClayLaunchers.size() << "\n";
        break;
    }

    default: break;
    }
}

void onCollision(physx::PxActor* a1, physx::PxActor* a2)
{
    PX_UNUSED(a1); PX_UNUSED(a2);
    // Sin colisiones en este prototipo
}

// ========== Main ==========
int main(int, const char* const*)
{
#ifndef OFFLINE_EXECUTION
    extern void renderLoop();
    renderLoop();
#else
    initPhysics(false);
    for (PxU32 i = 0; i < 600; ++i) stepPhysics(false, 1.0 / 60.0);
    cleanupPhysics(false);
#endif
    return 0;
}