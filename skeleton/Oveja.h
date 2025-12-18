// Oveja.h
#pragma once
#include <PxPhysicsAPI.h>
#include "core.hpp"
#include "RenderUtils.hpp"
#include "Vector3D.h"

class Oveja {
private:
    physx::PxRigidDynamic* cuerpo;   // cuerpo blanco
    physx::PxRigidDynamic* cabeza;   // cabeza negra (kinemática)
    RenderItem* cuerpoRender;
    RenderItem* cabezaRender;
    double radioCuerpo;
    double radioCabeza;
    double radioCuerpoBase;
    double radioCabezaBase;
    double masa;
    double escala = 1.0;
    physx::PxPhysics* physics;

public:
    Oveja(physx::PxPhysics* physics, physx::PxScene* scene, physx::PxMaterial* material,
        const Vector3& posicion,
        double radioCuerpo = 2.0,
        double radioCabeza = 1.0,
        double masa = 5.0);
    ~Oveja();

    physx::PxRigidDynamic* getCuerpo() { return cuerpo; }
    physx::PxRigidDynamic* getCabeza() { return cabeza; }
    void comer(double cantidad = 0.1f);
	void reconstruirShapes();

    void updateCabeza(); // sincroniza la posición de la cabeza con el cuerpo
};
