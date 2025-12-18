#pragma once
#include <PxPhysicsAPI.h>
#include "Vector3D.h"


class RigidForceGenerator
{
public:
    enum FGType {
        GENERIC,
        TIMED
    };

    RigidForceGenerator(FGType type = GENERIC)
        : typeID(type)
    {
    }

    FGType getType() const { return typeID; }

    // Esta es la función que se aplicará al sólido rígido
    virtual void updateForce(physx::PxRigidDynamic* body, double t) = 0;


private:
    FGType typeID;
};
