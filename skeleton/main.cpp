#include <ctype.h>
#include <PxPhysicsAPI.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>

#include "Vector3D.h"
#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"
#include "Particle.h"
#include "ParticleGenerator.h"
#include "ParticleForceRegistry.h"
#include "GravityForceGenerator.h"
#include "Trigo.h" // Nueva clase

std::string display_text = "This is a test";
Particle* p = NULL;

std::vector<ParticleGenerator*> emisores;
std::vector<double> tiempoRestanteEmisor;
std::vector<RenderItem*> plantas;
std::vector<Trigo*> cultivos; // Vector de plantas de trigo

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxMaterial* gMaterial = NULL;
PxPvd* gPvd = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
ContactReportCallback gContactReportCallback;

ParticleForceRegistry forceRegistry;
extern GravityForceGenerator* gravityEarth = new GravityForceGenerator(Vector3(0.0f, -9.8f, 0.0f));
extern GravityForceGenerator* gravityMoon = new GravityForceGenerator(Vector3(0.0f, -1.6f, 0.0f));


std::vector<Particle*> proyectiles;


// ============================================================
// Inicialización del motor de física
// ============================================================
void initPhysics(bool interactive)
{
	PX_UNUSED(interactive);

	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);

	// Crear 3 parcelas de tierra + plantas de trigo
	for (int i = 0; i < 3; i++) {
		float x = i * 10.0f - 10.0f;
		float y = 0.0f;
		float z = 0.0f;

		// Tierra
		PxShape* tierraShape = gPhysics->createShape(PxBoxGeometry(4.0f, 0.5f, 4.0f), *gMaterial);
		PxTransform* tierraTransform = new PxTransform(PxVec3(x, y, z));
		RenderItem* tierraItem = new RenderItem(tierraShape, tierraTransform, Vector4(0.4f, 0.25f, 0.1f, 1.0f));
		RegisterRenderItem(tierraItem);
	

		// Crear planta de trigo encima
		Trigo* planta = new Trigo(Vector3(x, y + 0.5f, z));
		cultivos.push_back(planta);
	}

	// Crear irrigadores (emisores de agua)
	std::vector<Vector3> posicionesBoquillas;
	for (int i = 0; i < 3; i++) {
		float x = i * 10.0f - 10.0f;
		float y = 10.0f;
		float z = 0.0f;

		PxShape* boquillaShape = gPhysics->createShape(PxBoxGeometry(1.0f, 1.0f, 1.0f), *gMaterial);
		PxTransform* boquillaTransform = new PxTransform(PxVec3(x, y, z));
		RenderItem* boquillaItem = new RenderItem(boquillaShape, boquillaTransform, Vector4(0.2f, 0.2f, 0.8f, 1.0f));
		RegisterRenderItem(boquillaItem);

		posicionesBoquillas.push_back(Vector3(x, y, z));
	}

	for (int i = 0; i < 3; i++) {
		ParticleGenerator* irrigador = new ParticleGenerator(
			posicionesBoquillas[i],
			Vector3(0, -10, 0), // hacia abajo
			1,
			50,
			0.3,
			2,
			Vector4(0.1, 0.3, 1.0, 1.0),
			0.2,
			false,
			0.0,
			0.1,
			0.3,
			0.1,
			0.1,
			2.0,
			0.0,
			2.0
		);
		irrigador->setActive(false);
		emisores.push_back(irrigador);
		tiempoRestanteEmisor.push_back(0.0);
	}
}


// ============================================================
// Step de física (por frame)
// ============================================================
void stepPhysics(bool interactive, double t)
{
	PX_UNUSED(interactive);

	forceRegistry.updateForces(t);

	// Integrar partículas existentes (proyectiles)
	for (int i = proyectiles.size() - 1; i >= 0; --i) {
		Particle* pr = proyectiles[i];
		if (pr != nullptr) {
			pr->integrate(t);
			if (!pr->isAlive()) {
				delete pr;
				proyectiles.erase(proyectiles.begin() + i);
			}
		}
	}

	// Actualizar emisores de agua (irrigadores)
	for (int i = 0; i < emisores.size(); i++) {
		if (emisores[i]->isActive()) {
			emisores[i]->update(t, proyectiles);
			tiempoRestanteEmisor[i] -= t;
			if (tiempoRestanteEmisor[i] <= 0.0) {
				emisores[i]->setActive(false);
				tiempoRestanteEmisor[i] = 0.0;
			}
		}
	}

	// Actualizar crecimiento del trigo (lógica visual) y sus generadores de partículas
	for (auto& trigo : cultivos) {
		// Actualiza la lógica de crecimiento / color del trigo
		trigo->update(t);

		// Actualiza el ParticleGenerator del trigo, pasando el vector global de proyectiles
		ParticleGenerator* gen = trigo->getParticleGenerator();
		if (gen && gen->isActive()) {
			gen->update(t, proyectiles);
		}
	}

	std::this_thread::sleep_for(std::chrono::microseconds(10));
}




// ============================================================
// Limpieza
// ============================================================
void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);


	forceRegistry.clear();

	delete gravityEarth;
	delete gravityMoon;
	gravityEarth = nullptr;
	gravityMoon = nullptr;


	for (auto& e : emisores)
		delete e;
	emisores.clear();
	tiempoRestanteEmisor.clear();

	for (auto& t : cultivos)
		delete t;
	cultivos.clear();

	plantas.clear();

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
		if (transport) transport->release();
		gPvd = nullptr;
	}

	if (gFoundation) {
		gFoundation->release();
		gFoundation = nullptr;
	}

	display_text.clear();
}



// ============================================================
// Colisiones 
// ============================================================
void onCollision(physx::PxActor* actor1, physx::PxActor* actor2)
{
	PX_UNUSED(actor1);
	PX_UNUSED(actor2);
}


// ============================================================
// Teclas
// ============================================================
void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case '1':
	case '2':
	case '3':
	{
		int index = key - '1';
		if (index >= 0 && index < emisores.size()) {
			emisores[index]->setActive(true);
			tiempoRestanteEmisor[index] = 2.0;
			cultivos[index]->regar(); // activar crecimiento del trigo
		}
		break;
	}
	default:
		break;
	}
}

void keyRelease(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(key);
	PX_UNUSED(camera);
}


// ============================================================
// Main
// ============================================================
int main(int, const char* const*)
{
#ifndef OFFLINE_EXECUTION 
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for (PxU32 i = 0; i < frameCount; i++)
		stepPhysics(false, 0.016);
	cleanupPhysics(false);
#endif

	return 0;
}
