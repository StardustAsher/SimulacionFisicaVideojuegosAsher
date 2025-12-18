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
#include "Trigo.h" 
#include "AnchoredSpringForceGenerator.h"
#include "SpringForceGenerator.h"
#include "TimeForceGenerator.h"
#include "Render/Camera.h"
#include "FloatForceGenerator.h"
#include "SolidSystem.h"
#include "RigidBodyGenerator.h"
#include "RigidForceRegistry.h"
#include "RigidWindForceGenerator.h"


SolidSystem solidSystem;	

Camera* camera = GetCamera();

bool mirandoTrigo = true;

Vector3 camTrigoPos(0.0f, 5.0f, -20.0f);
Vector3 camTrigoDir(0.0f, 0.0f, 1.0f);

Vector3 camOvejasPos(30.0f, 8.0f, 30.0f);
Vector3 camOvejasDir(-10.0f, -2.0f, -15.0f);

int trigoDisponible = 0;
std::string display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
Particle* p = NULL;

std::vector<ParticleGenerator*> emisores;
std::vector<RigidBodyGenerator*> emisoresRigid;
std::vector<double> tiempoRestanteEmisor;
std::vector<RenderItem*> plantas;
std::vector<Trigo*> cultivos; // Vector de plantas de trigo
SpringForceGenerator* ejemploSpring = nullptr;
AnchoredSpringForceGenerator* ejemploAnchoredSpring = nullptr;
Particle* floatingBox = nullptr;         // Cubo flotante
FloatForceGenerator* flotacion = nullptr; // Generador de flotación

ParticleForceRegistry forceRegistry;
RigidForceRegistry rigidForceRegistry;


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


extern GravityForceGenerator* gravityEarth = new GravityForceGenerator(Vector3(0.0f, -9.8f, 0.0f));
extern GravityForceGenerator* gravityMoon = new GravityForceGenerator(Vector3(0.0f, -1.6f, 0.0f));
extern WindForceGenerator* vientoSuave = new WindForceGenerator(
	Vector3(1.0, 0.0, 0.1),
	5.0
);

extern RigidWindForceGenerator* vientoRigido = new RigidWindForceGenerator(Vector3D(1.0, 0.0, 0.0), 100.0);


std::vector<Particle*> proyectiles;
std::vector<PxRigidDynamic*> rigidBodies;

// Variables globales para control de fuerzas
bool gravedadActiva = true;
bool vientoActivo = false;
bool vientoActivoRigid = true;
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

	

	// ===== SUELO =====
	PxRigidStatic* suelo = gPhysics->createRigidStatic(
		PxTransform(PxVec3(0.0f, -0.5f, 0.0f))
	);

	PxShape* sueloShape = gPhysics->createShape(
		PxBoxGeometry(50.0f, 0.5f, 50.0f),
		*gMaterial
	);

	suelo->attachShape(*sueloShape);
	gScene->addActor(*suelo);

	// Render
	RenderItem* sueloItem = new RenderItem(sueloShape, suelo, Vector4(0.3f, 0.8f, 0.3f, 1.0f));
	RegisterRenderItem(sueloItem);

	RegisterRenderItem(sueloItem);



	////Ejemplo muelle 1

	//Vector3 anchorPos(0.0f, 5.0f, 0.0f);

	//// crear la partícula 
	//Particle* particleMuella = new Particle(Vector3(0.0f, 3.0f, 0.0f), Vector3(0, 0, 0), Vector3(0, 0, 0), 100.0, 1.0, 0.99, 1, Vector4(1, 0, 0, 1), 0.3);

	//p = particleMuella; // para la f

	//// ancla
	//physx::PxShape* anchorShape = gPhysics->createShape(physx::PxBoxGeometry(0.2f, 0.2f, 0.2f), *gMaterial);
	//physx::PxTransform* anchorTransform = new physx::PxTransform(anchorPos);
	//RenderItem* anchorItem = new RenderItem(anchorShape, anchorTransform, Vector4(0.2, 0.8, 0.2, 1.0));
	//RegisterRenderItem(anchorItem);

	//// crear generador y registrarlo
	//AnchoredSpringForceGenerator* anchorSpring = new AnchoredSpringForceGenerator(anchorPos, 50.0 /*k*/, 2.0 /*rest len*/);
	//ejemploAnchoredSpring = anchorSpring;
	//forceRegistry.add(particleMuella, anchorSpring);

	//// Añadir gravedad
	//forceRegistry.add(particleMuella, gravityEarth);

	//proyectiles.push_back(particleMuella);


	////Ejemplo muelle 2

	//// crear dos partículas
	//Particle* pA = new Particle(Vector3(-1.0, 10.0, 0.0), Vector3(0, 0, 0), Vector3(0, 0, 0), 100.0, 1.0, 0.99, 1, Vector4(0.0, 0.7, 1.0, 1.0), 0.25);
	//Particle* pB = new Particle(Vector3(1.0, 10.0, 0.0), Vector3(0, 0, 0), Vector3(0, 0, 0), 100.0, 1.0, 0.99, 1, Vector4(0.8, 0.4, 0.1, 1.0), 0.25);

	//SpringForceGenerator* springAB = new SpringForceGenerator(pB, 10.0 /*k*/, 2.0 /*rest*/);
	//SpringForceGenerator* springBA = new SpringForceGenerator(pA, 10.0, 2.0);

	//// Registrar ambas direcciones
	//forceRegistry.add(pA, springAB);
	//forceRegistry.add(pB, springBA);


	////forceRegistry.add(pA, gravityEarth);
	//forceRegistry.add(pB, gravityEarth);

	//proyectiles.push_back(pA);
	//proyectiles.push_back(pB);

	////FLOTACION

	//floatingBox = new Particle(
	//	Vector3(0.0f, 2.0f, 0.0f),
	//	Vector3(0.0f, 0.0f, 0.0f),
	//	Vector3(0.0f, 0.0f, 0.0f),
	//	1000.0,
	//	1.0,
	//	0.99,
	//	2,
	//	Vector4(1.0, 0.5, 0.2, 1.0),
	//	0.5
	//);

	//proyectiles.push_back(floatingBox);

	//// Flotación
	//flotacion = new FloatForceGenerator(
	//	1.0f,        // altura
	//	0.125f,      // volumen
	//	1000.0f,     // densidad agua
	//	floatingBox
	//);

	//forceRegistry.add(floatingBox, flotacion);
	//forceRegistry.add(floatingBox, gravityEarth);


	//GENERADOR DE CUBOS RIGIDOS

	


	// Crear un generador de cubos
	RigidBodyGenerator* generadorCubos = new RigidBodyGenerator(
		gPhysics,      // PhysX physics
		gScene,        // PhysX scene
		Vector3D(0, 10, 0), // posición inicial (10 unidades sobre el suelo)
		Vector3D(0, 0, 0),  // velocidad inicial
		2.0,           // 2 cubos por segundo
		10.0,          // densidad
		gMaterial      // material
	);

	emisoresRigid.push_back(generadorCubos);

	//JUEGO
	
	// //Crear 3 parcelas de tierra + plantas de trigo
	//for (int i = 0; i < 3; i++) {
	//	float x = i * 10.0f - 10.0f;
	//	float y = 0.0f;
	//	float z = 0.0f;

	//	 //Tierra
	//	PxShape* tierraShape = gPhysics->createShape(PxBoxGeometry(4.0f, 0.5f, 4.0f), *gMaterial);
	//	PxTransform* tierraTransform = new PxTransform(PxVec3(x, y, z));
	//	RenderItem* tierraItem = new RenderItem(tierraShape, tierraTransform, Vector4(0.4f, 0.25f, 0.1f, 1.0f));
	//	RegisterRenderItem(tierraItem);

	//	 //Planta de trigo
	//	Trigo* planta = new Trigo(Vector3(x, y + 0.5f, z));
	//	cultivos.push_back(planta);
	//}

	// //Crear irrigadores (emisores de agua)
	//std::vector<Vector3> posicionesBoquillas;
	//for (int i = 0; i < 3; i++) {
	//	float x = i * 10.0f - 10.0f;
	//	float y = 10.0f;
	//	float z = 0.0f;

	//	PxShape* boquillaShape = gPhysics->createShape(PxBoxGeometry(1.0f, 1.0f, 1.0f), *gMaterial);
	//	PxTransform* boquillaTransform = new PxTransform(PxVec3(x, y, z));
	//	RenderItem* boquillaItem = new RenderItem(boquillaShape, boquillaTransform, Vector4(0.2f, 0.2f, 0.8f, 1.0f));
	//	RegisterRenderItem(boquillaItem);

	//	posicionesBoquillas.push_back(Vector3(x, y, z));
	//}

	//for (int i = 0; i < 3; i++) {
	//	ParticleGenerator* irrigador = new ParticleGenerator(
	//		posicionesBoquillas[i],
	//		Vector3(0, -10, 0),
	//		1, 50, 0.5, 2,
	//		Vector4(0.1, 0.3, 1.0, 1.0),
	//		0.2, false, 0.0, 0.1, 0.3, 0.1, 0.1, 2.0, 0.0, 2.0
	//	);
	//	irrigador->setActive(false);
	//	emisores.push_back(irrigador);
	//	tiempoRestanteEmisor.push_back(0.0);
	//}

	//// --- Crear un corral simple ---
	//for (int i = 0; i < 4; i++) {
	//	float x = 25.0f + (i % 2) * 6.0f;
	//	float z = 15.0f + (i / 2) * 8.0f;
	//	float y = 1.0f;
	//	// Vallas

	//	PxShape* vallaShape = gPhysics->createShape(PxBoxGeometry(3.0f, 1.0f, 0.2f), *gMaterial);
	//	PxTransform* vallaTransform = new PxTransform(PxVec3(x, y, z));
	//	RenderItem* vallaItem = new RenderItem(vallaShape, vallaTransform, Vector4(0.5, 0.3, 0.1, 1));
	//	RegisterRenderItem(vallaItem);
	//}

	//// --- Ovejas (3 esferas blancas por ejemplo) ---
	//for (int i = 0; i < 3; i++) {
	//	float x = 20.0f + (rand() % 5 - 2);
	//	float y = 1.0f;
	//	float z = 5.0f + (rand() % 5 - 2);

	//	PxShape* sheep = gPhysics->createShape(PxSphereGeometry(0.8f), *gMaterial);
	//	PxTransform* ts = new PxTransform(PxVec3(x, y, z));
	//	RenderItem* r = new RenderItem(sheep, ts, Vector4(1, 1, 1, 1));
	//	RegisterRenderItem(r);
	//}



	
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
	forceRegistry.removeExpired(t);

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


	if (vientoActivoRigid) {
		rigidForceRegistry.updateForces(t);
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

	for(int i = 0; i < emisoresRigid.size(); i++) {
		emisoresRigid[i]->update(t, rigidBodies);
	}

	// Actualizar trigo
	for (auto& trigo : cultivos) {
		trigo->update(t);
		ParticleGenerator* gen = trigo->getParticleGenerator();
		if (gen && gen->isActive()) {
			gen->update(t, proyectiles);
		}
	}

	gScene->simulate(t);
	gScene->fetchResults(true);


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
// elimina la duplicación bool mirandoTrigo = true; deja sólo una declaración global
// bool mirandoTrigo = true;  // declarar una vez

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
	}
	break;

	case 'R':
	{
		for (auto& trigo : cultivos) {
			if (trigo->estaMaduro()) {
				trigo->reset();
				trigoDisponible++;
				std::cout << "Trigo recolectado. Total disponible: " << trigoDisponible << std::endl;
			}
		}
		display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
	}
	break;

	case 'T':
	{
		if (trigoDisponible > 0) {
			trigoDisponible--;
			double masa = 0.008;
			double vel = 380.0;
			double velAjustada = 30.0;
			dispararProyectil(pos, dir, masa, vel, velAjustada, 5, Vector4(1.0, 1.0, 0.1, 1.0));
		}
		display_text = "Trigo disponible: " + std::to_string(trigoDisponible);
	}
	break;

	case 'Z':
		gravedadActiva = !gravedadActiva;
		std::cout << (gravedadActiva ? "Gravedad activada" : "Gravedad desactivada") << std::endl;
		break;

	case 'X':
		vientoActivo = !vientoActivo;
		std::cout << (vientoActivo ? "Viento activado" : "Viento desactivado") << std::endl;
		break;

	case '+':
		if (ejemploSpring) {
			double k = ejemploSpring->getK();
			k *= 1.2;
			ejemploSpring->setK(k);
			std::cout << "k aumentado a " << k << std::endl;
		}
		if (ejemploAnchoredSpring) {
			double k = ejemploAnchoredSpring->getK();
			k *= 1.2;
			ejemploAnchoredSpring->setK(k);
			std::cout << "k aumentado a " << k << std::endl;
		}
		break;

	case '-':
		if (ejemploSpring) {
			double k = ejemploSpring->getK();
			k /= 1.2;
			ejemploSpring->setK(k);
			std::cout << "k reducido a " << k << std::endl;
		}
		if (ejemploAnchoredSpring) {
			double k = ejemploAnchoredSpring->getK();
			k /= 1.2;
			ejemploAnchoredSpring->setK(k);
			std::cout << "k reducido a " << k << std::endl;
		}
		break;

	case 'F':
	{
		Vector3 demoForce(500.0, 0.0, 0.0);
		TimedForceGenerator* tfg = new TimedForceGenerator(demoForce, 0.15);
		forceRegistry.add(p, tfg);
		std::cout << "Fuerza temporal aplicada" << std::endl;
	}
	break;

	case 'C':
	{
		Camera* camera = GetCamera();
		if (mirandoTrigo)
		{
			Vector3 dirNorm = camOvejasDir;
			dirNorm.normalize(); // Normaliza el Vector3
			camera->setEye(PxVec3(camOvejasPos.x, camOvejasPos.y, camOvejasPos.z));
			camera->setDir(PxVec3(dirNorm.x, dirNorm.y, dirNorm.z));
		}
		else
		{
			Vector3 dirNorm = camTrigoDir;
			dirNorm.normalize();
			camera->setEye(PxVec3(camTrigoPos.x, camTrigoPos.y, camTrigoPos.z));
			camera->setDir(PxVec3(dirNorm.x, dirNorm.y, dirNorm.z));
		}
		mirandoTrigo = !mirandoTrigo;

	}
	break;
	case 'M': // aumentar masa
	{
		double m = floatingBox->getMass();
		floatingBox->setMass(m * 1.2);
		std::cout << "Masa: " << floatingBox->getMass() << std::endl;
	}
	break;

	case 'N': // reducir masa
	{
		double m = floatingBox->getMass();
		floatingBox->setMass(m / 1.2);
	}
	break;

	






	default:
		break;
	} // fin switch
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