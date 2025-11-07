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
#include "WindForceGenerator.h"
#include "Trigo.h" // Nueva clase

int trigoDisponible = 10;
std::string display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
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
extern WindForceGenerator* vientoSuave = new WindForceGenerator(
	Vector3(1.0, 0.0, 0.1),
	5.0
);

std::vector<Particle*> proyectiles;

// Variables globales para control de fuerzas
bool gravedadActiva = true;
bool vientoActivo = false;

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

		// Planta de trigo
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
			Vector3(0, -10, 0),
			1, 50, 0.5, 2,
			Vector4(0.1, 0.3, 1.0, 1.0),
			0.2, false, 0.0, 0.1, 0.3, 0.1, 0.1, 2.0, 0.0, 2.0
		);
		irrigador->setActive(false);
		emisores.push_back(irrigador);
		tiempoRestanteEmisor.push_back(0.0);
	}
}

// ============================================================
// Función para disparar proyectiles
// ============================================================
void dispararProyectil(const Vector3& pos, const Vector3& dirOriginal, double masa, double vel, double velAjustada, int tipo, const Vector4& color)
{
	Vector3 dir = dirOriginal;
	dir.normalize();
	double energia = 0.5 * masa * vel * vel;

	double masaAjustada = (2.0 * energia) / (velAjustada * velAjustada);
	Vector3 vectorVel = dir * velAjustada;

	double size = 0.5;
	switch (tipo)
	{
	case 1: size = 0.2; break;
	case 2: size = 0.4; break;
	case 3: size = 0.8; break;
	case 4: size = 0.05; break;
	case 5: size = 0.3; break;
	default: break;
	}

	Particle* nuevo = new Particle(pos, vectorVel, Vector3(0.0, 0.0, 0.0), 10.0, masaAjustada, 0.99, 2, color, size);
	proyectiles.push_back(nuevo);

	forceRegistry.add(nuevo, gravityEarth);
}

// ============================================================
// Step de física
// ============================================================
void stepPhysics(bool interactive, double t)
{
	PX_UNUSED(interactive);

	// Solo aplica fuerzas activas
	ForceGenerator* gravedadActual = gravedadActiva ? gravityEarth : gravityMoon;
	forceRegistry.updateForcesConditional(t, true, vientoActivo, gravedadActual, vientoSuave);


	// Integrar partículas
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

	// Actualizar emisores
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

	// Actualizar trigo
	for (auto& trigo : cultivos) {
		trigo->update(t);
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
	delete vientoSuave;
	gravityEarth = nullptr;
	gravityMoon = nullptr;
	vientoSuave = nullptr;

	for (auto& e : emisores) delete e;
	emisores.clear();
	tiempoRestanteEmisor.clear();

	for (auto& t : cultivos) delete t;
	cultivos.clear();

	plantas.clear();

	if (gScene) gScene->release();
	if (gDispatcher) gDispatcher->release();
	if (gPhysics) gPhysics->release();
	if (gPvd) {
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		if (transport) transport->release();
	}
	if (gFoundation) gFoundation->release();

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
	Vector3 pos = camera.p;
	Vector3 dir = -camera.q.getBasisVector2(); // Hacia delante
	dir.normalize();

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
			cultivos[index]->regar();
		}
		break;
	}
	case 'E':
	{
		for (auto& trigo : cultivos) {
			if (trigo->estaMaduro()) {
				trigo->reset();
				trigoDisponible++;
				std::cout << "Trigo recolectado. Total disponible: " << trigoDisponible << std::endl;
			}
		}
		display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
		break;
	}
	case 'T': // Disparar trigo 
	{
		if (trigoDisponible > 0) {
			trigoDisponible--;
			double masa = 0.008;
			double vel = 380.0;
			double velAjustada = 30.0;
			dispararProyectil(pos, dir, masa, vel, velAjustada, 5, Vector4(1.0, 1.0, 0.1, 1.0));
		}
		display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
		break;
	}
	case 'Z': // Activar/desactivar gravedad
	{
		gravedadActiva = !gravedadActiva;
		std::cout << (gravedadActiva ? "Gravedad activada" : "Gravedad desactivada") << std::endl;
		break;
	}
	case 'X': // Activar/desactivar viento
	{
		vientoActivo = !vientoActivo;
		std::cout << (vientoActivo ? "Viento activado" : "Viento desactivado") << std::endl;
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


//Otros tipos de disparos por si acaso

//case '1': //Pistola
//{
//	double masa = 0.008;
//	double vel = 380.0;
//	double velAjustada = 100.0;
//	dispararProyectil(pos, dir, masa, vel, velAjustada, 1, Vector4(1.0, 0.0, 0.0, 1.0));
//	break;
//}
//
//case '2': //Bala de ca��n ligera
//{
//	double masa = 6.0;
//	double vel = 250.0;
//	double velAjustada = 60.0;
//	dispararProyectil(pos, dir, masa, vel, velAjustada, 2, Vector4(0.0, 0.0, 1.0, 1.0));
//	break;
//}
//
//case '3': //Proyectil de tanque 
//{
//	double masa = 20.0;
//	double vel = 1500.0;
//	double velAjustada = 120.0;
//	dispararProyectil(pos, dir, masa, vel, velAjustada, 3, Vector4(0.5, 0.5, 0.5, 1.0));
//	break;
//}
//
//case '4': // Laser 
//{
//	double masa = 0.000001;
//	double vel = 3e8;
//	double velAjustada = 300.0;
//	double size = 0.05;           // Muy pequeño
//	Particle* laser = new Particle(
//		pos,
//		dir * velAjustada,
//		Vector3(0, 0, 0),
//		3.0,
//		masa,
//		1.0,
//		1,
//		Vector4(1.0, 1.0, 0.0, 1.0),
//		size
//	);
//	proyectiles.push_back(laser);
//	break;
//}