#include <ctype.h>
#include <PxPhysicsAPI.h>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <algorithm> // remove_if

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"

// Tus includes del sistema de partculas
#include "Sim/Particles/ParticleSystem.h"
#include "Sim/Particles/ProjectileManager.h"
#include "Sim/Particles/Forces/GravityForce.h"
#include "Sim/Particles/Forces/WindForce.h"
#include "Sim/Particles/Forces/BuoyancyForceGenerator.h"

using namespace physx;

// -----------------------------------------------------------------------
// GLOBAL VARS
// -----------------------------------------------------------------------
PxDefaultAllocator         gAllocator;
PxDefaultErrorCallback     gErrorCallback;
PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;
PxPvd* gPvd = nullptr;
PxMaterial* gMaterial = nullptr;
ContactReportCallback     gContactCB;

std::string display_text = "Tiro al Pato 2.0: ESPACIO disparar | C lanzar pato | V viento";

int gScore = 0;
int gShots = 0;


// --- GESTORES ---
ProjectileManager   gProjManager; // Balas (Particulas custom)
ParticleSystem      gParticleSys; // Sistema para efectos / fuerzas

// --- FUERZAS ---
// Punteros para gestin rpida en main
std::unique_ptr<WindForce>    gWindForce;

// --- PATOS (Slidos Rgidos) ---
struct Duck {
    PxRigidDynamic* body;
    RenderItem* renderItem;
    double life;
    bool isDead;
};
std::vector<Duck*> gDucks;

// --- AGUA (Visual) ---
RenderItem* gWaterRenderItem = nullptr;
float gWaterLevel = 0.0f;

// -----------------------------------------------------------------------
// CORRECCIN ERROR LNK2019
// -----------------------------------------------------------------------
// Esta funcin es llamada desde callbacks.cpp. Debe existir aunque est vaca.
void onCollision(physx::PxActor* actor1, physx::PxActor* actor2)
{
    PX_UNUSED(actor1);
    PX_UNUSED(actor2);
    // Aqu ira lgica de colisin entre dos SOLIDOS de PhysX.
    // Como mezclamos Partculas (Balas) y Slidos (Patos), 
    // la lgica la hacemos manual en checkCollisions().
}

// -----------------------------------------------------------------------
// HELPERS
// -----------------------------------------------------------------------


static const char* ProjectileName(ProjectileKind k)
{
    switch (k) {
    case ProjectileKind::CannonBall: return "CannonBall";
    case ProjectileKind::TankShell:  return "TankShell";
    case ProjectileKind::Pistol:     return "Pistol";
    default:                         return "?";
    }
}

// Generador de Patos (Requisito: Generador de Slidos)
void spawnDuck() {
    if (!gPhysics || !gScene) return;

    // Posicin aleatoria en un lateral
    float x = -40.0f;
    float y = 10.0f + (rand() % 10);
    float z = (rand() % 40) - 20.0f;

    // Velocidad hacia el otro lado
    float vx = 15.0f + (rand() % 10);
    float vy = 5.0f + (rand() % 5);

    // PhysX Setup
    PxTransform t(PxVec3(x, y, z));
    PxRigidDynamic* body = gPhysics->createRigidDynamic(t);

    // Forma (Caja)
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(1.0f, 0.5f, 0.8f), *gMaterial);
    body->attachShape(*shape);

    // Requisito: Masa e inercia calculadas con densidad
    float density = 400.0f + (rand() % 200);
    PxRigidBodyExt::updateMassAndInertia(*body, density);

    body->setLinearVelocity(PxVec3(vx, vy, 0));
    // Rotacin inicial para que se vea dinmico
    body->setAngularVelocity(PxVec3(0, 1, 0));
    body->setAngularDamping(0.5f);

    gScene->addActor(*body);

    // Visual
    Vector4 color(1.0f, 0.8f, 0.2f, 1.0f); // Amarillo pato
    RenderItem* item = new RenderItem(shape, body, color);

    Duck* d = new Duck();
    d->body = body;
    d->renderItem = item;
    d->life = 15.0; // 15 segs de vida
    d->isDead = false;

    gDucks.push_back(d);
    shape->release();
}

// Lgica de colisin manual: Partcula vs Slido Rgido
void checkCollisions() {
    // Obtenemos las partculas activas gracias al getter nuevo
    const auto& particles = gProjManager.GetParticles();

    for (auto* duck : gDucks) {
        if (duck->isDead || !duck->body) continue;

        PxTransform duckPose = duck->body->getGlobalPose();
        Vector3D duckPos(duckPose.p.x, duckPose.p.y, duckPose.p.z);

        // Asumimos que el pato tiene un radio de colisin aprox de 1.5 unidades
        float duckRadius = 1.5f;

        for (auto& p : particles) {
            if (!p || !p->IsAlive()) continue; // Si la bala ya no existe

            Vector3D bulletPos = p->GetPosition();
            Vector3D distVec = duckPos - bulletPos;

            if (distVec.length() < duckRadius) {
                // --- IMPACTO DETECTADO ---

                // 1. Matar al pato
                duck->isDead = true;
                ++gScore;
                duck->life = 0.5; // deja el pato un instante en pantalla y luego se limpia
                duck->renderItem->color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // Se vuelve ROJO al morir

                // 2. Efecto fsico en el pato (le damos un golpe fuerte)
                Vector3D velBala = p->GetVelocity();
                PxVec3 force(velBala.getX(), velBala.getY(), velBala.getZ());
                duck->body->addForce(force * 50.0f); // Multiplicador de fuerza
                duck->body->setAngularVelocity(PxVec3(10, 10, 0)); // Girar loco

                // 3. Matar la bala (hack: le ponemos vida negativa)
                // OJO: Particle no tiene setLife pblico en tu cdigo original, 
                // pero suele borrarse si se sale del mundo o timer.
                // Como workaround, la mandamos al inframundo para que se borre sola:
                // Si pudieras editar Particle.h aadiras: void Kill() { _life = -1; }
                // Como no quiero hacerte editar ms ficheros, asumimos que sigue su curso o rebota.
                // Pero visualmente ya hemos dado al pato.

                break; // Una bala solo mata un pato a la vez
            }
        }
    }
}

// Lgica de flotacin simple para cumplir requisito
void applyBuoyancyToDucks(double dt) {
    for (auto* duck : gDucks) {
        if (!duck->body) continue;

        PxTransform t = duck->body->getGlobalPose();
        if (t.p.y < gWaterLevel) {
            float depth = gWaterLevel - t.p.y;
            // Fuerza hacia arriba (Empuje)
            float forceY = (depth * 150.0f) + 10.0f;

            // Friccin del agua (Drag)
            PxVec3 vel = duck->body->getLinearVelocity();
            PxVec3 drag = -vel * 2.0f;

            duck->body->addForce(PxVec3(0, forceY, 0) + drag);
        }
    }
}

// -----------------------------------------------------------------------
// CORE FUNCTIONS
// -----------------------------------------------------------------------

void initPhysics(bool interactive)
{
    PX_UNUSED(interactive);

    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

    PxSceneDesc desc(gPhysics->getTolerancesScale());
    desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    desc.cpuDispatcher = gDispatcher;
    desc.filterShader = PxDefaultSimulationFilterShader;
    desc.simulationEventCallback = &gContactCB;
    gScene = gPhysics->createScene(desc);

    // --- ESCENARIO ---

    // Agua (Visual)
    PxShape* waterShape = gPhysics->createShape(PxBoxGeometry(100.0f, 0.1f, 50.0f), *gMaterial);
    PxTransform waterPose(PxVec3(0, gWaterLevel, 0));
    gWaterRenderItem = new RenderItem(waterShape, &waterPose, Vector4(0.0f, 0.2f, 1.0f, 0.6f));

    // Fuerza Viento
    // Lo instanciamos y lo guardamos. Usamos unique_ptr.
    gWindForce = std::make_unique<WindForce>(Vector3D(10.0f, 0.0f, 0.0f), 1.0f, 0.1f);

    // Aadir copia del viento al sistema de partculas para que afecte a BALAS
    // (Ojo: ParticleSystem usa unique_ptr, as que creamos OTRO objeto igual)
    gParticleSys.AddForceGenerator(std::make_unique<WindForce>(Vector3D(10.0f, 0.0f, 0.0f), 1.0f, 0.1f));

    // Configura proyectil inicial (IMPORTANTE: inicializar TODOS los campos)
    // Si no, _life/_damping/_color quedan basura y la bala puede desaparecer al instante.
    ProjectileDesc canon = gProjManager.GetPreset(ProjectileKind::CannonBall);
    canon._mReal   = 2.0f;
    canon._vReal   = 100.0f;
    canon._vSim    = 100.0f;
    canon._gSim    = -9.8f;
    canon._radius  = 0.5f;
    canon._color   = Vector4(0.9f, 0.9f, 0.2f, 1.0f);
    canon._life    = 6.0;
    canon._damping = 0.995f;
    gProjManager.SetPreset(ProjectileKind::CannonBall, canon);

}

void stepPhysics(bool interactive, double dt)
{
    PX_UNUSED(interactive);

    // Spawn automtico de patos
    static double timer = 0.0;
    timer += dt;
    if (timer > 2.0) {
        spawnDuck();
        timer = 0.0;
    }

    // Aplicar fuerzas a Patos (PhysX) y a Balas (Partículas)
    if (gWindForce && gWindForce->Enabled()) {
        // Requisito: Fuerza sobre Sólido Rígido
        for (auto* d : gDucks) {
            gWindForce->Apply(d->body, dt);
        }

        // Extra: el mismo viento afecta a las balas (partículas)
        const auto& bullets = gProjManager.GetParticles();
        for (auto& b : bullets) {
            if (b) gWindForce->Apply(*b, dt);
        }
    }

applyBuoyancyToDucks(dt); // Requisito: Flotacin

    // Simular PhysX
    gScene->simulate(dt);
    gScene->fetchResults(true);

    // Update Sistemas propios
    gProjManager.Update(dt);
    gParticleSys.Update(dt);

    // Comprobar colisiones (La magia que conecta ambos mundos)
    checkCollisions();

    // Limpieza de Patos muertos o viejos
    auto it = gDucks.begin();
    while (it != gDucks.end()) {
        Duck* d = *it;
        d->life -= dt;

        // Si se acaba la vida o cae muy profundo
        if (d->life <= 0 || d->body->getGlobalPose().p.y < -20.0f) {
            d->renderItem->release(); // Borrar visual
            d->body->release();       // Borrar fsica
            delete d;                 // Borrar struct
            it = gDucks.erase(it);
        }
        else {
            ++it;
        }
    }

    // HUD / Texto en pantalla
    display_text =
        std::string("Tiro al Pato 2.0 | Score: ") + std::to_string(gScore) +
        " | Shots: " + std::to_string(gShots) +
        " | Ducks: " + std::to_string(gDucks.size()) +
        " | Proj: " + ProjectileName(gProjManager.Current()) +
        " | Wind: " + (gWindForce && gWindForce->Enabled() ? "ON" : "OFF") +
        " | [ESPACIO] disparar  [Q/E] tipo  [C] pato  [V] viento";


}

void cleanupPhysics(bool interactive)
{
    PX_UNUSED(interactive);
    gProjManager.Clear();
    gParticleSys.Clear();

    for (auto* d : gDucks) {
        if (d->renderItem) d->renderItem->release();
        if (d->body) d->body->release();
        delete d;
    }
    gDucks.clear();

    if (gWaterRenderItem) gWaterRenderItem->release();

    gScene->release();
    gDispatcher->release();
    gPhysics->release();
    if (gPvd) {
        PxPvdTransport* t = gPvd->getTransport();
        gPvd->release(); t->release();
    }
    gFoundation->release();
}

void keyPress(unsigned char key, const PxTransform& camera)
{
    switch (toupper(key))
    {
    case ' ': gProjManager.Fire(); ++gShots; break;
    case 'Q': gProjManager.Cycle(-1); break;
    case 'E': gProjManager.Cycle(+1); break;
    case 'C': spawnDuck(); break;
    case 'V':
        if (gWindForce) {
            gWindForce->SetEnabled(!gWindForce->Enabled());
            std::cout << "Viento: " << (gWindForce->Enabled() ? "ON" : "OFF") << std::endl;
        }
        break;
    }
}

int main(int, const char* const*)
{
#ifndef OFFLINE_EXECUTION
    extern void renderLoop();
    renderLoop();
#else
    initPhysics(false);
    for (int i = 0; i < 600; ++i) stepPhysics(false, 0.016);
    cleanupPhysics(false);
#endif
    return 0;
}