#include "RigidBodyGenerator.h"
#include "RenderUtils.hpp"


RigidBodyGenerator::RigidBodyGenerator(
    PxPhysics* physics,
    PxScene* scene,
    Vector3D position,
    Vector3D meanVelocity,
    double emitRate,
    double density,
    PxMaterial* material
)
    : physics(physics),
    scene(scene),
    position(position),
    meanVelocity(meanVelocity),
    emitRate(emitRate),
    density(density),
    material(material)
{
}

void RigidBodyGenerator::update(double dt, std::vector<PxRigidDynamic*>& outBodies) {
    timeAccumulator += dt;
    double interval = 1.0 / emitRate;

    while (timeAccumulator >= interval) {
        timeAccumulator -= interval;
        emit(outBodies);
    }
}

void RigidBodyGenerator::emit(std::vector<PxRigidDynamic*>& outBodies) {
    if (!active) return;

    // Transform inicial
    PxTransform transform(PxVec3(position.x, position.y, position.z));

    PxRigidDynamic* body = physics->createRigidDynamic(transform);

    // Forma (caja por ejemplo)
    PxShape* shape = physics->createShape(
        PxBoxGeometry(0.5f, 0.5f, 0.5f),
        *material
    );

    body->attachShape(*shape);

    // Masa e inercia AUTOMÁTICAS
    PxRigidBodyExt::updateMassAndInertia(*body, density);

    // Velocidad inicial
    body->setLinearVelocity(
        PxVec3(meanVelocity.x, meanVelocity.y, meanVelocity.z)
    );

    body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false); // asegurarte que está activada

    scene->addActor(*body);
    outBodies.push_back(body);

    // Render
    RenderItem* r = new RenderItem(
        shape,
        body,
        Vector4(0.7f, 0.2f, 0.2f, 1.0f)
    );
	
    RegisterRenderItem(r);
}
