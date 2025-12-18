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
	#include "Oveja.h"


	struct ColisionTrigoOveja
	{
		PxRigidDynamic* trigo;
	};

	struct CuboConMuelles {
		Particle* cubo;                // Partícula principal (cubo)
		std::vector<Particle*> muelles; // Partículas coloreadas unidas al cubo
		std::vector<SpringForceGenerator*> springs; // Muelles
	};

	std::vector<CuboConMuelles> cubosEsquinas;

	std::vector<ColisionTrigoOveja> colisionesPendientes;
	std::vector<ColisionTrigoOveja> paraEliminar;

	SolidSystem solidSystem;

	Camera* camera = GetCamera();

	bool mirandoTrigo = true;
	bool decoracionCreada = false; 

	int comidasOveja = 0;

	Vector3 camTrigoPos(0.0f, 5.0f, -30.0f);
	Vector3 camTrigoDir(0.0f, 0.0f, 1.0f);

	Vector3 camOvejasPos(0.0f, 5.0f, -30.0f);
	Vector3 camOvejasDir(-90.0f, 0.0f, 1.0f);

	std::vector<Particle*> particulasDeco;               // Partículas colgantes
	std::vector<AnchoredSpringForceGenerator*> muellesDeco; // Muelles de cada partícula

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

	Particle* anclaIzq;
	Particle* anclaDer;

	ParticleForceRegistry forceRegistry;
	RigidForceRegistry rigidForceRegistry;
	Oveja* oveja1 = nullptr;

	float separation = 0.4f; // separación horizontal entre ojos
	float offsetX = 0.5f; // desplazamiento lateral (izq/derecha)
	float offsetY = 0.2f; // altura relativa
	// delante de la cabeza


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
	extern RigidWindForceGenerator* vientoArriba = new RigidWindForceGenerator(Vector3D(0.0, 1.0, 0.0), 500.0);


	extern RigidWindForceGenerator* vientoRigido = new RigidWindForceGenerator(Vector3D(1.0, 0.0, 0.0), 100.0);


	std::vector<Particle*> proyectiles;
	std::vector<PxRigidDynamic*> rigidBodies;

	struct TrigoRigid
	{
		PxRigidDynamic* actor;
		RenderItem* render;
	};

	std::vector<TrigoRigid> trigosRigidos;


	// Variables globales para control de fuerzas
	bool gravedadActiva = true;
	bool vientoActivo = false;
	bool vientoActivoRigid = true;
	bool vientoArribaAplicado = false;
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

		Camera* camera = GetCamera();
		camera->setEye(PxVec3(camTrigoPos.x, camTrigoPos.y, camTrigoPos.z));
		camera->setDir(PxVec3(camTrigoDir.x, camTrigoDir.y, camTrigoDir.z));
		mirandoTrigo = true;

	

		// ===== SUELO =====
		PxRigidStatic* suelo = gPhysics->createRigidStatic(
			PxTransform(PxVec3(0.0f, -0.5f, 0.0f))
		);

		PxShape* sueloShape = gPhysics->createShape(
			PxBoxGeometry(100.0f, 0.5f, 100.0f),
			*gMaterial
		);

		suelo->attachShape(*sueloShape);
		gScene->addActor(*suelo);

		// Render
		RenderItem* sueloItem = new RenderItem(sueloShape, suelo, Vector4(0.3f, 0.8f, 0.3f, 1.0f));

		RegisterRenderItem(sueloItem);


		//JUEGO
	
		 //Crear 3 parcelas de tierra + plantas de trigo
		for (int i = 0; i < 3; i++) {
			float x = i * 10.0f - 10.0f;
			float y = 0.0f;
			float z = 0.0f;

			 //Tierra
			PxShape* tierraShape = gPhysics->createShape(PxBoxGeometry(4.0f, 0.5f, 4.0f), *gMaterial);
			PxTransform* tierraTransform = new PxTransform(PxVec3(x, y, z));
			RenderItem* tierraItem = new RenderItem(tierraShape, tierraTransform, Vector4(0.4f, 0.25f, 0.1f, 1.0f));
			RegisterRenderItem(tierraItem);

			 //Planta de trigo
			Trigo* planta = new Trigo(Vector3(x, y + 0.5f, z));
			cultivos.push_back(planta);
		}

		 //Crear irrigadores (emisores de agua)
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

		// ---------------- CORRAL ----------------

		PxMaterial* matCorral = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

		float corralHalfSize = 11.0f;   // ahora bastante más grande
		float wallHeight = 2.0f;       // altura aumentada
		float wallThickness = 0.3f;    // un poco más gruesas

		// Posición central del corral, frente a la cámara de ovejas
		Vector3 corralCenter(-25.0f, 2.0f, -30.0f);

		// Color marrón claro
		Vector4 colorPared(0.6f, 0.4f, 0.2f, 1.0f);

		// Pared frontal
		PxRigidStatic* paredFrontal = gPhysics->createRigidStatic(
			PxTransform(PxVec3(corralCenter.x, wallHeight / 2.0f, corralCenter.z + corralHalfSize))
		);
		PxShape* shapeFrontal = gPhysics->createShape(PxBoxGeometry(corralHalfSize, wallHeight*4, wallThickness), *matCorral);
		paredFrontal->attachShape(*shapeFrontal);
		gScene->addActor(*paredFrontal);
		RegisterRenderItem(new RenderItem(shapeFrontal, paredFrontal, colorPared));

		// Pared trasera
		PxRigidStatic* paredTrasera = gPhysics->createRigidStatic(
			PxTransform(PxVec3(corralCenter.x, wallHeight / 2.0f, corralCenter.z - corralHalfSize))
		);
		PxShape* shapeTrasera = gPhysics->createShape(PxBoxGeometry(corralHalfSize, wallHeight*4, wallThickness), *matCorral);
		paredTrasera->attachShape(*shapeTrasera);
		gScene->addActor(*paredTrasera);
		RegisterRenderItem(new RenderItem(shapeTrasera, paredTrasera, colorPared));

		// Pared izquierda
		PxRigidStatic* paredIzq = gPhysics->createRigidStatic(
			PxTransform(PxVec3(corralCenter.x - corralHalfSize, wallHeight / 2.0f, corralCenter.z))
		);
		PxShape* shapeIzq = gPhysics->createShape(PxBoxGeometry(wallThickness, wallHeight*4, corralHalfSize), *matCorral);
		paredIzq->attachShape(*shapeIzq);
		gScene->addActor(*paredIzq);
		RegisterRenderItem(new RenderItem(shapeIzq, paredIzq, colorPared));

		// Pared derecha
		PxRigidStatic* paredDer = gPhysics->createRigidStatic(
			PxTransform(PxVec3(corralCenter.x + corralHalfSize, wallHeight / 2.0f, corralCenter.z))
		);
		PxShape* shapeDer = gPhysics->createShape(PxBoxGeometry(wallThickness, wallHeight, corralHalfSize), *matCorral);
		paredDer->attachShape(*shapeDer);
		gScene->addActor(*paredDer);
		RegisterRenderItem(new RenderItem(shapeDer, paredDer, colorPared));

		// Suelo interno del corral (opcional)
		PxRigidStatic* sueloCorral = gPhysics->createRigidStatic(
			PxTransform(PxVec3(corralCenter.x, -0.01f, corralCenter.z))
		);
		PxShape* shapeSueloCorral = gPhysics->createShape(PxBoxGeometry(corralHalfSize, 0.01f, corralHalfSize), *matCorral);
		sueloCorral->attachShape(*shapeSueloCorral);
		gScene->addActor(*sueloCorral);
		RegisterRenderItem(new RenderItem(shapeSueloCorral, sueloCorral, Vector4(0.8f, 0.7f, 0.5f, 1.0f))); // suelo ligeramente diferente

		// ---------------- OVEJA ----------------
	
		oveja1 = new Oveja(gPhysics, gScene, gMaterial, Vector3(-20.0f, 2.0f, -25.0f), 3.0, 1.0, 5.0);

		// --- OJOS DE LA OVEJA COMO PARTICULAS CON MUELLES ---

		PxVec3 cabezaPos = oveja1->getCabeza()->getGlobalPose().p;
		float separation = 0.4f;   // separación entre ojos
		float offsetX = 0.5f;      // delante de la cabeza
		float offsetY = 0.2f;      // altura relativa

		// Partículas de ojos
		Particle* ojoIzq = new Particle(
			Vector3(cabezaPos.x - offsetX, cabezaPos.y + offsetY, cabezaPos.z),
			Vector3(0.0, 0.0, 0.0),
			Vector3(0.0, 0.0, 0.0),
			1000.0,
			0.01,
			0.99,
			0,
			Vector4(1.0, 1.0, 1.0, 1.0),
			1.0
		);
		ojoIzq->setEternal(true);
		Particle* ojoDer = new Particle(
			Vector3(cabezaPos.x + offsetX, cabezaPos.y + offsetY, cabezaPos.z),
			Vector3(0.0, 0.0, 0.0),
			Vector3(0.0, 0.0, 0.0),
			1000.0,
			0.01,
			0.99,
			0,
			Vector4(1.0, 1.0, 1.0, 1.0),
			1.0
		);
		ojoDer->setEternal(true);


		// Anclas en la cabeza (partículas “inmóviles”)
		 anclaIzq = new Particle(
			Vector3(cabezaPos.x - separation, cabezaPos.y + offsetY, cabezaPos.z),
			Vector3(0, 0, 0),
			Vector3(0, 0, 0),
			1000.0,
			0.0,
			0.0,
			0,
			Vector4(),
			0.1
		);

		 anclaIzq->setEternal(true);

		 anclaDer = new Particle(
			Vector3(cabezaPos.x + separation, cabezaPos.y + offsetY, cabezaPos.z),
			Vector3(0, 0, 0),
			Vector3(0, 0, 0),
			1000.0,
			0.0,
			0.0,
			0,
			Vector4(),
			0.1
		);

		 anclaDer->setEternal(true);

		// Crear muelles
		SpringForceGenerator* springIzq = new SpringForceGenerator(anclaIzq, 50.0, 0.0);
		SpringForceGenerator* springDer = new SpringForceGenerator(anclaDer, 50.0, 0.0);

		// Registrar fuerzas
		forceRegistry.add(ojoIzq, springIzq);
		forceRegistry.add(ojoDer, springDer);

		forceRegistry.add(ojoIzq, gravityEarth);
		forceRegistry.add(ojoDer, gravityEarth);


		// Guardar las partículas globalmente para actualizar cada frame
		proyectiles.push_back(ojoIzq);
		proyectiles.push_back(ojoDer);


		//DECO
// Parámetros de la cuadrícula
		int gridSize = 3;
		float spacing = 8.0f;       // separación entre anclas
		float anchorHeight = 10.0f; // altura de las anclas
		Vector3 startPos = corralCenter; // centro de la cuadrícula

		// Colores más variados para las bolas
		Vector4 colorPairs[2][9] = {
			{Vector4(1, 0, 0, 1), Vector4(1, 0.5, 0, 1), Vector4(1, 1, 0, 1),
			 Vector4(0, 1, 0, 1), Vector4(0, 1, 0.5, 1), Vector4(0, 1, 1, 1),
			 Vector4(0, 0, 1, 1), Vector4(0.5, 0, 1, 1), Vector4(1, 0, 1, 1)}, // bola 1

			{Vector4(0, 1, 1, 1), Vector4(0, 0, 1, 1), Vector4(0.5, 0, 1, 1),
			 Vector4(1, 0, 1, 1), Vector4(1, 0, 0, 1), Vector4(1, 0.5, 0, 1),
			 Vector4(1, 1, 0, 1), Vector4(0, 1, 0.5, 1), Vector4(0, 1, 0, 1)}  // bola 2
		};

		int index = 0; // para asignar colores a cada ancla

		for (int i = 0; i < gridSize; ++i) {
			for (int j = 0; j < gridSize; ++j) {
				// Posición del ancla
				Vector3 anchorPos(
					startPos.x + (i - 1) * spacing,
					anchorHeight,
					startPos.z + (j - 1) * spacing
				);

				// Crear forma del ancla
				physx::PxShape* anchorShape = gPhysics->createShape(
					physx::PxBoxGeometry(0.2f, 0.2f, 0.2f), *gMaterial
				);
				physx::PxTransform* anchorTransform = new physx::PxTransform(anchorPos);
				RenderItem* anchorItem = new RenderItem(anchorShape, anchorTransform, Vector4(0.2, 0.8, 0.2, 1.0));
				RegisterRenderItem(anchorItem);

				// Crear 2 bolas por ancla con colores vibrantes
				for (int b = 0; b < 2; ++b) {
					Vector3 particlePos = anchorPos + Vector3(0.0f, -1.0f - b * 0.5f, 0.0f);

					Particle* particleMuella = new Particle(
						particlePos,
						Vector3(0, 0, 0),
						Vector3(0, 0, 0),
						300.0f,
						1.0f,
						0.92f,
						1,
						colorPairs[b][index], // color más variado
						0.3f
					);

					// Crear resorte
					AnchoredSpringForceGenerator* anchorSpring = new AnchoredSpringForceGenerator(
						anchorPos,
						15.0f,
						1.5f + b * 0.5f
					);
					forceRegistry.add(particleMuella, anchorSpring);
					forceRegistry.add(particleMuella, gravityEarth);

					proyectiles.push_back(particleMuella);
				}

				index++; // siguiente par de colores
			}
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

	void dispararTrigoRigid(const Vector3& pos, const Vector3& dirOriginal, double masa, double vel, double size)
	{
		Vector3 dir = dirOriginal;
		dir.normalize();

		PxTransform transform(PxVec3(pos.x, pos.y, pos.z));
		PxRigidDynamic* trigoRigid = gPhysics->createRigidDynamic(transform);

		PxShape* shape = gPhysics->createShape(PxSphereGeometry(size), *gMaterial);
		trigoRigid->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*trigoRigid, masa);
		trigoRigid->setLinearVelocity(PxVec3(dir.x * vel, dir.y * vel, dir.z * vel));

		gScene->addActor(*trigoRigid);

		RenderItem* renderTrigo = new RenderItem(shape, trigoRigid, Vector4(1, 1, 0, 1));
		trigosRigidos.push_back({ trigoRigid, renderTrigo });
	}




	// ============================================================
	// Step de física
	// ============================================================
	void stepPhysics(bool interactive, double t)
	{
		PX_UNUSED(interactive);

		if (comidasOveja >= 5 && !vientoArribaAplicado)
		{
			rigidForceRegistry.add(oveja1->getCuerpo(), vientoArriba);
			vientoArribaAplicado = true;
			display_text = "La oveja astronauta ha iniciado su misión. ¡¡¡Enhorabuena!!!";
			
		}
		


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

		oveja1->updateCabeza();

		PxVec3 cabezaPosActual = oveja1->getCabeza()->getGlobalPose().p;
		anclaIzq->setPosition(Vector3(cabezaPosActual.x - offsetX, cabezaPosActual.y + offsetY, cabezaPosActual.z));
		anclaDer->setPosition(Vector3(cabezaPosActual.x + offsetX, cabezaPosActual.y + offsetY, cabezaPosActual.z));


		for (auto& pr : particulasDeco) {
			pr->integrate(t);
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

		// ================= PROCESAR COLISIONES PENDIENTES =================
		// ================= PROCESAR COLISIONES PENDIENTES =================
		for (const auto& col : colisionesPendientes)
		{
			PxRigidDynamic* trigo = col.trigo;

			auto it = std::find_if(trigosRigidos.begin(), trigosRigidos.end(),
				[&](const TrigoRigid& t) { return t.actor == trigo; });

			if (it == trigosRigidos.end())
				continue;

			// La oveja come
			oveja1->comer(0.2);
			comidasOveja++;
			std::cout << "La oveja ha comido trigo" << std::endl;

			gScene->removeActor(*it->actor);
			it->actor->release();
			DeregisterRenderItem(it->render);
			delete it->render;
			trigosRigidos.erase(it);
		}




		// limpiar cola
		colisionesPendientes.clear();





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

		for (auto& p : particulasDeco) delete p;
		particulasDeco.clear();

		for (auto& m : muellesDeco) delete m;
		muellesDeco.clear();


		display_text.clear();
	}

	// ============================================================
	// Colisiones
	// ============================================================
	void onCollision(physx::PxActor* actor1, physx::PxActor* actor2)
	{
		if (!oveja1) return;

		PxActor* ovejaCuerpo = oveja1->getCuerpo();
		PxActor* ovejaCabeza = oveja1->getCabeza();

		PxRigidDynamic* trigo = nullptr;

		if (actor1 == ovejaCuerpo || actor1 == ovejaCabeza)
			trigo = actor2->is<PxRigidDynamic>();
		else if (actor2 == ovejaCuerpo || actor2 == ovejaCabeza)
			trigo = actor1->is<PxRigidDynamic>();

		if (!trigo) return;

		auto it = std::find_if(trigosRigidos.begin(), trigosRigidos.end(),
			[&](const TrigoRigid& t) { return t.actor == trigo; });

		if (it == trigosRigidos.end()) return;

		colisionesPendientes.push_back({ trigo });
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
				double masa = 0.5;        // ajustable para rigido
				double vel = 20.0;        // ajustable
				double size = 0.2;        // radio de la esfera
				dispararTrigoRigid(pos, dir, masa, vel, size);
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