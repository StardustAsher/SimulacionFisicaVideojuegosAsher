#pragma once
#include <PxPhysicsAPI.h>
#include <vector>

using namespace physx;

class SolidSystem {
public:
    void add(PxRigidDynamic* body) {
        solids.push_back(body);
    }

    const std::vector<PxRigidDynamic*>& getSolids() const {
        return solids;
    }

private:
    std::vector<PxRigidDynamic*> solids;
};
