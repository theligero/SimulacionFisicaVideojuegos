#include <PxPhysicsAPI.h>
#include <vector>
#include <memory>
#include <iostream>

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

#include "Sim/Particles/Particle.h"
#include "Sim/Particles/Forces/GravityForce.h"
#include "Sim/Particles/Forces/Springs/SpringForceGenerator.h"
#include "Sim/Particles/Forces/Springs/AnchoredSpringFG.h"
#include "Sim/Particles/Forces/BuoyancyForceGenerator.h"

using namespace physx;

// -------------------------------------------------------------
// PhysX core globals (lo que espera RenderUtils / callbacks)
// -------------------------------------------------------------

static PxDefaultAllocator      gAllocator;
static PxDefaultErrorCallback  gErrorCallback;

PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;
PxPvd* gPvd = nullptr;

PxMaterial* gMaterial = nullptr;
std::string              display_text = "P4 - Muelles y flotación";

// Callback de colisiones (lo usa callbacks.hpp)
ContactReportCallback    gContactReportCallback;

// -------------------------------------------------------------
// Partículas y generadores de fuerza de la práctica 4
// -------------------------------------------------------------

std::vector<std::unique_ptr<Particle>> gParticles;

// fuerzas
std::unique_ptr<GravityForce>          gGravity;
bool                                   gUseGravity = true;
bool                                   gUseBuoyancy = true;

std::unique_ptr<AnchoredSpringFG>      gAnchoredSpring;
std::unique_ptr<SpringForceGenerator>  gSpringAB;
std::unique_ptr<BuoyancyForceGenerator> gBuoyancy;

// punteros “rápidos” a las partículas
Particle* gMassAnchored = nullptr; // masa con muelle anclado
Particle* gMassA = nullptr; // masa A del muelle AB
Particle* gMassB = nullptr; // masa B del muelle AB
Particle* gFloating = nullptr; // objeto que flota
Particle* gWaterParticle = nullptr; // "nivel" del agua

// agua para pintar la superficie
PxTransform  gWaterPose(PxVec3(10.0f, 0.0f, 0.0f));
PxShape* gWaterShape = nullptr;
RenderItem* gWaterItem = nullptr;

// -------------------------------------------------------------
// Helpers
// -------------------------------------------------------------

Particle* createSphereParticle(const Vector3D& pos,
    float radius,
    const Vector4& color,
    float mass)
{
    auto p = std::make_unique<Particle>(
        pos,
        Vector3D{ 0.0f, 0.0f, 0.0f }, // vel inicial
        radius,
        color,
        -1.0,                         // vida infinita
        0.99f,                        // damping
        Vector3D{ 0.0f, 0.0f, 0.0f }, // la gravedad se mete como fuerza
        mass
    );

    Particle* raw = p.get();
    gParticles.emplace_back(std::move(p));
    return raw;
}

void clearCustomScene()
{
    // destruye partículas (liberan RenderItem + shape)
    gParticles.clear();

    gMassAnchored = gMassA = gMassB = gFloating = nullptr;

    if (gWaterItem) {
        gWaterItem->release();
        gWaterItem = nullptr;
    }
    if (gWaterShape) {
        gWaterShape->release();
        gWaterShape = nullptr;
    }
}

void createCustomScene()
{
    clearCustomScene();

    // ---------- MUELLE ANCLADO ----------
    gMassAnchored = createSphereParticle(
        Vector3D{ -10.0f, 8.0f, 0.0f },   // pos inicial
        0.5f,
        Vector4{ 1.0f, 1.0f, 0.0f, 1.0f }, // amarillo
        2.0f
    );

    // ---------- MUELLE ENTRE DOS MASAS ----------
    gMassA = createSphereParticle(
        Vector3D{ 0.0f, 5.0f, 0.0f },
        0.5f,
        Vector4{ 1.0f, 0.0f, 0.0f, 1.0f }, // rojo
        1.0f
    );

    gMassB = createSphereParticle(
        Vector3D{ 4.0f, 5.0f, 0.0f },
        0.5f,
        Vector4{ 0.0f, 0.0f, 1.0f, 1.0f }, // azul
        1.0f
    );

    // ---------- OBJETO QUE FLOTA ----------
    gFloating = createSphereParticle(
        Vector3D{ 10.0f, 3.0f, 0.0f },
        0.7f,
        Vector4{ 0.0f, 1.0f, 0.0f, 1.0f }, // verde
        2.0f
    );

    // ---------- SUPERFICIE DEL AGUA (solo visual) ----------
    gWaterShape = CreateShape(PxBoxGeometry(10.0f, 0.1f, 10.0f));
    gWaterItem = new RenderItem(gWaterShape, &gWaterPose,
        Vector4{ 0.0f, 0.3f, 1.0f, 0.4f });

    // ---------- GENERADORES DE FUERZA ----------
    gGravity = std::make_unique<GravityForce>(Vector3D{ 0.0f, -9.8f, 0.0f });

    // muelle anclado (ancla por encima de la masa)
    Vector3D anchorPos{ -10.0f, 10.0f, 0.0f };
    gAnchoredSpring = std::make_unique<AnchoredSpringFG>(
        15.0f,   // k
        2.0f,     // longitud de reposo
        anchorPos, gMassAnchored
    );

    // muelle entre A y B
    gSpringAB = std::make_unique<SpringForceGenerator>(
        10.0f,  // k
        4.0f,    // longitud de reposo
        gMassA, gMassB
    );

    gWaterParticle = createSphereParticle(
        Vector3D{ 10.0f, 0.0f, 0.0f },
        0.05f,
        Vector4{ 0.0f, 0.3f, 1.0f, 0.4f },
        0.0f
    );

    // flotación (agua en y = 0)
    float volume = 1.0f;
    float liquidHeight = 0.0f;
    float density = 1000.0f;
    gBuoyancy = std::make_unique<BuoyancyForceGenerator>(
        gFloating, gWaterParticle, liquidHeight, volume, density
    );
}

// -------------------------------------------------------------
// Funciones requeridas por el framework
// -------------------------------------------------------------

void initPhysics(bool interactive)
{
    PX_UNUSED(interactive);

    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    gPvd = PxCreatePvd(*gFoundation);

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
        PxTolerancesScale(), true, gPvd);

    // *** NUEVO: material por defecto para CreateShape ***
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f); // gravedad la hacemos nosotros
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    sceneDesc.simulationEventCallback = &gContactReportCallback;

    gScene = gPhysics->createScene(sceneDesc);

    setupDefaultRenderState();

    createCustomScene();
}

// *** IMPORTANTE ***
// Firma con (bool, double) para que enlace con RenderUtils.
void stepPhysics(bool interactive, double dt)
{
    PX_UNUSED(interactive);

    // actualizamos PhysX (aunque no usemos actores dinámicos)
    gScene->simulate(static_cast<PxReal>(dt));
    gScene->fetchResults(true);

    // 1) limpiar fuerzas acumuladas
    for (auto& p : gParticles)
        if (p) p->ClearForces();

    // 2) aplicar fuerzas
    if (gGravity && gUseGravity) {
        for (auto& p : gParticles)
            if (p) gGravity->Apply(*p, dt);
    }

    if (gAnchoredSpring && gMassAnchored)
        gAnchoredSpring->Apply(*gMassAnchored, dt);

    if (gSpringAB && gMassA && gMassB) {
        gSpringAB->Apply(*gMassA, dt);
        gSpringAB->Apply(*gMassB, dt);
    }

    if (gBuoyancy && gFloating)
        gBuoyancy->Apply(*gFloating, dt);

    // 3) integrar partículas
    for (auto& p : gParticles)
        if (p) p->Integrate(dt);
}

void cleanupPhysics(bool interactive)
{
    PX_UNUSED(interactive);

    clearCustomScene();

    if (gMaterial) {
        gMaterial->release();
        gMaterial = nullptr;
    }

    if (gScene) {
        gScene->release();
        gScene = nullptr;
    }

    if (gDispatcher) {
        gDispatcher->release();
        gDispatcher = nullptr;
    }

    if (gPhysics) {
        gPhysics->release();
        gPhysics = nullptr;
    }

    if (gPvd) {
        PxPvdTransport* transport = gPvd->getTransport();
        gPvd->release();
        gPvd = nullptr;
        transport->release();
    }

    if (gFoundation) {
        gFoundation->release();
        gFoundation = nullptr;
    }
}

void keyPress(unsigned char key, const PxTransform& camera)
{
    switch (std::tolower(key))
    {
    case 'r':
        std::cout << "Reset escena practica 4\n";
        createCustomScene();
        break;

    case 'g':
        gUseGravity = !gUseGravity;
        std::cout << "Gravedad: " << (gUseGravity ? "ON" : "OFF") << "\n";
        break;

    case 'b':
        gUseBuoyancy = !gUseBuoyancy;
        std::cout << "Empuje (buoyancy): " << (gUseBuoyancy ? "ON" : "OFF") << "\n";
        break;

        // Pequeños impulsos para ver el efecto de los muelles/ flotación
    case '1':
        if (gMassAnchored)
        {
            // Dale un pequeño empujón horizontal
            Vector3D v = gMassAnchored->GetVelocity();
            v = v + Vector3D{ 2.0f, 0.0f, 0.0f };
            // No tienes setVelocity, pero puedes usar AddForce un frame:
            gMassAnchored->AddForce(Vector3D{ 20.0f, 0.0f, 0.0f });
        }
        break;

    case '2':
        if (gMassA && gMassB)
        {
            gMassA->AddForce(Vector3D{ 20.0f, 0.0f, 0.0f });
            gMassB->AddForce(Vector3D{ -20.0f, 0.0f, 0.0f });
        }
        break;

    case '3':
        if (gFloating)
        {
            // Lo empujamos un poco hacia abajo para que entre más en el agua
            gFloating->AddForce(Vector3D{ 0.0f, -30.0f, 0.0f });
        }
        break;
    case '+':
        if (gAnchoredSpring) {
            gAnchoredSpring->SetK(gAnchoredSpring->GetK() + 5.0f);
            std::cout << "K muelle anclado: " << gAnchoredSpring->GetK() << '\n';
        }
        break;
    case '-':
        if (gAnchoredSpring) {
            gAnchoredSpring->SetK(gAnchoredSpring->GetK() - 5.0f);
            std::cout << "K muelle anclado: " << gAnchoredSpring->GetK() << '\n';
        }
        break;
    case 'm': // Aumentar masa
        if (gFloating) gFloating->SetMass(gFloating->GetMass() + 0.5f);
        break;
    case 'v': // Aumentar volumen (afecta al empuje)
        if (gBuoyancy) gBuoyancy->SetVolume(gBuoyancy->GetVolume() + 1.0f);
        break;

    default:
        break;
    }
}

// Para esta práctica no necesitamos nada especial en colisiones
void onCollision(PxActor* actor1, PxActor* actor2)
{
    PX_UNUSED(actor1);
    PX_UNUSED(actor2);
}

// -------------------------------------------------------------
// main
// -------------------------------------------------------------

int main()
{
    std::cout << "Practica 4 - Muelles y Flotacion\n";
    std::cout << "Controles:\n"
        << "  R : reset escena\n"
        << "  G : activar/desactivar gravedad\n"
        << "  B : activar/desactivar empuje (flotacion)\n"
        << "  1 : empujar masa del muelle anclado\n"
        << "  2 : empujar las dos masas unidas por muelle\n"
        << "  3 : empujar objeto flotante hacia abajo\n"
        << "  M : aumentar la masa\n"
        << "  V : aumentar volumen\n";

#ifndef OFFLINE_EXECUTION
    extern void renderLoop();
    renderLoop();
#else
    initPhysics(false);
    const double dt = 1.0 / 60.0;
    for (PxU32 i = 0; i < 600; ++i)
        stepPhysics(false, dt);
    cleanupPhysics(false);
#endif

    return 0;
}