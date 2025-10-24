#include <ctype.h>

#include <PxPhysicsAPI.h>

#include <vector>
#include "Vector3D.h"
#include <chrono>
#include <thread>

#include "core.hpp"
#include "RenderUtils.hpp"
#include "callbacks.hpp"
#include "Particle.h"
#include "ParticleGenerator.h"
#include "ParticleForceRegistry.h"
#include "GravityForceGenerator.h"


#include <iostream>

std::string display_text = "This is a test";
Particle* p = NULL;
std::vector<ParticleGenerator*> emisores;

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

extern ParticleForceRegistry forceRegistry;
extern GravityForceGenerator* gravityEarth = new GravityForceGenerator(Vector3(0.0f, -9.8f, 0.0f));
extern GravityForceGenerator* gravityMoon = new GravityForceGenerator(Vector3(0.0f, -1.6f, 0.0f));


std::vector<Particle*> proyectiles;


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

	

	////Esfera
	//PxShape* sphereShape = gPhysics->createShape(PxSphereGeometry(2.0f), *gMaterial);
	//PxTransform* sphereTransform = new PxTransform(PxVec3(0.0f, 0.0f, 0.0f));
	//Vector3 color = Vector3(0.0f, 0.0f, 0.0f); 
	//
	//RenderItem* sphereRenderItem = new RenderItem(sphereShape, sphereTransform, Vector4(color.x, color.y, color.z, 1.0f));

	//RegisterRenderItem(sphereRenderItem);



	// Particula random
	p = new Particle(
		Vector3(0.0, 10.0, 0.0),
		Vector3(5.0, 0.0, 0.0),
		Vector3(0.0, 0.0, 0.0), 
		20.0, 1.0, 0.99, 1, Vector4(1.0, 0.0, 0.0, 1.0)
	);
	proyectiles.push_back(p);

	// Registrar la partícula en el ParticleForceRegistry
	forceRegistry.add(p, gravityEarth);

	// Crear emisores
	emisores.push_back(new ParticleGenerator(
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 30.0f, 0.0f),
		1,
		30.0, 10.0, 1,
		Vector4(0.0f, 0.5f, 0.8f, 1.0f),
		false, 5.0,
		0.2, 0.0, 1.0, 0.5
	));

	//EsferasVector
	/*
	Vector3D posicionEsfera1 = Vector3D(10.0f, 0.0f, 0.0f);
	Vector3D posicionEsfera2 = Vector3D(0.0f, 10.0f, 0.0f);
	Vector3D posicionEsfera3 = Vector3D(0.0f, 0.0f, 10.0f);

	RenderItem* esfera1 = new RenderItem(sphereShape, new PxTransform(PxVec3(posicionEsfera1.x, posicionEsfera1.y, posicionEsfera1.z)), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	RegisterRenderItem(esfera1);
	RenderItem* esfera2 = new RenderItem(sphereShape, new PxTransform(PxVec3(posicionEsfera2.x, posicionEsfera2.y, posicionEsfera2.z)), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	RegisterRenderItem(esfera2);
	RenderItem* esfera3 = new RenderItem(sphereShape, new PxTransform(PxVec3(posicionEsfera3.x, posicionEsfera3.y, posicionEsfera3.z)), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	RegisterRenderItem(esfera3);*/

	
	
	//p = new Particle(Vector3(0.0, 10.0, .0), Vector3(0.0, 0.5, 0.0), Vector3(0.0, -1.0, 0.0), 20.0, 1.0, 0.99);

	}


// Function to configure what happens in each step of physics
// interactive: true if the game is rendering, false if it offline
// t: time passed since last call in milliseconds
void stepPhysics(bool interactive, double t)
{
	PX_UNUSED(interactive);

	forceRegistry.updateForces(t); // actualizar fuerzas

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

	for (auto& e : emisores)
		e->update(t, proyectiles);
	
	gScene->simulate(t);
	gScene->fetchResults(true);

	std::this_thread::sleep_for(std::chrono::microseconds(10));
}

// Function to clean data
// Add custom code to the begining of the function
void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);

	forceRegistry.clear();

	delete gravityEarth;
	delete gravityMoon;

	for (auto& e : emisores) delete e;
	emisores.clear();

	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	gFoundation->release();
}




void onCollision(physx::PxActor* actor1, physx::PxActor* actor2)
{
	PX_UNUSED(actor1);
	PX_UNUSED(actor2);
}

void dispararProyectil(Vector3 pos, Vector3 dir, double masa, double vel, double velAjustada,
	int shape = 1, Vector4 color = Vector4(1.0, 0.0, 0.0, 1.0))
{
	dir.normalize();

	double energia = 0.5 * masa * vel * vel;
	double masaAjustada = (2.0 * energia) / (velAjustada * velAjustada);
	Vector3 vectorVel = dir * velAjustada;

	
	Particle* nuevo = new Particle(pos, vectorVel, Vector3(0, 0, 0), 10.0, masaAjustada, 0.99, shape, color);
	proyectiles.push_back(nuevo);

	
	forceRegistry.add(nuevo, gravityEarth);
}



void keyPress(unsigned char key, const PxTransform& camera)
{
	Vector3 pos = camera.p;
	Vector3 dir = -camera.q.getBasisVector2(); // Hacia delante
	dir.normalize();

	switch (toupper(key))
	{
	case '1': //Pistola
	{
		double masa = 0.008;         
		double vel = 380.0;           
		double velAjustada = 100.0;   
		dispararProyectil(pos, dir, masa, vel, velAjustada, 1, Vector4(1.0, 0.0, 0.0, 1.0));
		break;
	}

	case '2': //Bala de cañón ligera
	{
		double masa = 6.0;            
		double vel = 250.0;           
		double velAjustada = 60.0;    
		dispararProyectil(pos, dir, masa, vel, velAjustada, 2, Vector4(0.0, 0.0, 1.0, 1.0));
		break;
	}

	case '3': //Proyectil de tanque 
	{
		double masa = 20.0;           
		double vel = 1500.0;         
		double velAjustada = 120.0;   
		dispararProyectil(pos, dir, masa, vel, velAjustada, 3, Vector4(0.5, 0.5, 0.5, 1.0));
		break;
	}

	case '4': //Láser 
	{
		double masa = 0.000001;       
		double vel = 3e8;             
		double velAjustada = 300.0;   
		Particle* laser = new Particle(pos, dir * velAjustada, Vector3(0, 0, 0), 3.0, masa, 1.0, 1, Vector4(1.0, 1.0, 0.0, 1.0));
		proyectiles.push_back(laser);
		break;
	}

	case 'Q': //Partícula estática — solo cae
	{
		double masa = 1.0;            
		double vel = 0.0;             // sin movimiento
		double velAjustada = 0.0;     
		dispararProyectil(pos, dir, masa, vel, velAjustada, 1, Vector4(0.0, 1.0, 0.0, 1.0));
		break;
	}

	default:
		break;
	}
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