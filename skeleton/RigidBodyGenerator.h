#pragma once
#include <PxPhysicsAPI.h>
#include <vector>
#include "Vector3D.h"

using namespace physx;

class RigidBodyGenerator {
public:
    RigidBodyGenerator(
        PxPhysics* physics,
        PxScene* scene,
        Vector3D position, // Vector3 debe estar definido en Vector3D.h
        Vector3D meanVelocity,
        double emitRate,
        double density,
        PxMaterial* material
    );

    void update(double dt, std::vector<PxRigidDynamic*>& outBodies);
    void setActive(bool a) { active = a; }

private:
    void emit(std::vector<PxRigidDynamic*>& outBodies);

    PxPhysics* physics;
    PxScene* scene;
    PxMaterial* material;

    Vector3D position;
    Vector3D meanVelocity;

    double emitRate;
    double density;
    double timeAccumulator = 0.0;
    bool active = true;
};
