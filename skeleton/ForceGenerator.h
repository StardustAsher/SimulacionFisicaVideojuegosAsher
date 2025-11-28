#pragma once

#include "Particle.h"

class ForceGenerator
{
public:

    enum FGType {
        GENERIC,
        TIMED
    };

    ForceGenerator(FGType type = GENERIC)
        : typeID(type)
    {
    }

    FGType getType() const {
        return typeID;
    }
    // -------------------------------------------------------------

    virtual void updateForce(Particle* particle, double t) = 0;

private:
    FGType typeID;   
};
