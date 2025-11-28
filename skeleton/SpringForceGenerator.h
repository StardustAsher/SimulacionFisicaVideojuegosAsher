#pragma once
#include "ForceGenerator.h"
#include "Particle.h"

// Muelle entre dos partículas
class SpringForceGenerator : public ForceGenerator {
public:
    SpringForceGenerator(Particle* other, double k, double restLength)
        : otherParticle(other), k(k), restLength(restLength) {
    }

    void updateForce(Particle* particle, double t) override;

    void setK(double k_) { k = k_; }
    double getK() const { return k; }
    void setRestLength(double r) { restLength = r; }
private:
    Particle* otherParticle;
    double k;
    double restLength;
};
