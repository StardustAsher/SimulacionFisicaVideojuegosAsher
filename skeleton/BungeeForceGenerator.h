#pragma once
#include "ForceGenerator.h"
#include "Particle.h"

class BungeeForceGenerator : public ForceGenerator {
public:
    BungeeForceGenerator(Particle* other, double k, double restLength)
        : otherParticle(other), k(k), restLength(restLength) {
    }

    void updateForce(Particle* particle, double t) override;

    void setK(double k_) { k = k_; }
private:
    Particle* otherParticle;
    double k;
    double restLength;
};
