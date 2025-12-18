// Oveja.cpp
#include "Oveja.h"
#include <PxPhysicsAPI.h>
#include "core.hpp"

using namespace physx;

Oveja::Oveja(PxPhysics* physics_, PxScene* scene, PxMaterial* material,
    const Vector3& posicion,
    double rCuerpo, double rCabeza, double m)
    : radioCuerpo(rCuerpo), radioCabeza(rCabeza), radioCuerpoBase(rCuerpo),
    radioCabezaBase(rCabeza), masa(m), physics(physics_)
{
    if (!physics || !scene || !material) return;

    // --- Crear cuerpo dinámico (blanco) ---
    cuerpo = physics->createRigidDynamic(PxTransform(PxVec3(posicion.x, posicion.y, posicion.z)));
    PxShape* cuerpoShape = physics->createShape(PxSphereGeometry(radioCuerpo), *material);
    cuerpo->attachShape(*cuerpoShape);
    PxRigidBodyExt::updateMassAndInertia(*cuerpo, float(masa));
    scene->addActor(*cuerpo);
    cuerpoRender = new RenderItem(cuerpoShape, cuerpo, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    RegisterRenderItem(cuerpoRender);

    // --- Crear cabeza (negra) como kinemática delante del cuerpo ---
    cabeza = physics->createRigidDynamic(
        PxTransform(PxVec3(posicion.x, posicion.y, posicion.z + radioCuerpo + radioCabeza))
    );
    cabeza->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true); // no afectada por gravedad
    PxShape* cabezaShape = physics->createShape(PxSphereGeometry(radioCabeza), *material);
    cabeza->attachShape(*cabezaShape);
    scene->addActor(*cabeza);
    cabezaRender = new RenderItem(cabezaShape, cabeza, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    RegisterRenderItem(cabezaRender);
}

Oveja::~Oveja()
{
    if (cuerpo) {
        cuerpo->getScene()->removeActor(*cuerpo);
        cuerpo->release();
        cuerpo = nullptr;
    }
    if (cabeza) {
        cabeza->getScene()->removeActor(*cabeza);
        cabeza->release();
        cabeza = nullptr;
    }
    if (cuerpoRender) {
        delete cuerpoRender;
        cuerpoRender = nullptr;
    }
    if (cabezaRender) {
        delete cabezaRender;
        cabezaRender = nullptr;
    }
}

// Sincronizar cabeza con el cuerpo (frente)
void Oveja::updateCabeza()
{
    if (!cuerpo || !cabeza) return;

    PxTransform cuerpoPose = cuerpo->getGlobalPose();
    // Offset en Z hacia delante
    PxVec3 nuevaPos = cuerpoPose.p + PxVec3(radioCuerpo + radioCabeza, 0.0f,0.0f);
    cabeza->setKinematicTarget(PxTransform(nuevaPos, PxQuat(PxIdentity)));
}
void Oveja::comer(double cantidad)
{
    escala += cantidad;

    // recalcular radios
    radioCuerpo = radioCuerpoBase * escala;
    radioCabeza = radioCabezaBase * escala;

    reconstruirShapes();

    std::cout << "Oveja ha comido! Escala: " << escala << std::endl;
}


void Oveja::reconstruirShapes()
{
    // --- CUERPO ---
    PxShape* oldShape = nullptr;
    cuerpo->getShapes(&oldShape, 1);
    cuerpo->detachShape(*oldShape);
    PxMaterial* mat = oldShape->getMaterialFromInternalFaceIndex(0);
    oldShape->release();

    PxShape* newBodyShape = physics->createShape(
        PxSphereGeometry(radioCuerpo), *mat
    );
    cuerpo->attachShape(*newBodyShape);

    cuerpoRender->shape = newBodyShape;
    PxRigidBodyExt::updateMassAndInertia(*cuerpo, float(masa));

    // --- CABEZA ---
    PxShape* oldHeadShape = nullptr;
    cabeza->getShapes(&oldHeadShape, 1);
    cabeza->detachShape(*oldHeadShape);
    oldHeadShape->release();

    PxShape* newHeadShape = physics->createShape(
        PxSphereGeometry(radioCabeza), *mat
    );
    cabeza->attachShape(*newHeadShape);

    cabezaRender->shape = newHeadShape;
}



