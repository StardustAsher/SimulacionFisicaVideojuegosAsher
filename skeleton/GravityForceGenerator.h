#pragma once
#include "ForceGenerator.h"

class GravityForceGenerator : public ForceGenerator
{
private:

    Vector3 gravity;

public:

    GravityForceGenerator(const Vector3& g);

    void updateForce(Particle* p, double t) override;
};

