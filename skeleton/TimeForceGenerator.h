#pragma once
#include "ForceGenerator.h"
#include "Particle.h"

// Fuerza constante aplicada durante (duration) segundos
class TimedForceGenerator : public ForceGenerator {
public:
    TimedForceGenerator(const Vector3& force, double duration)
        : ForceGenerator(FGType::TIMED), 
        force(force),
        remaining(duration)
    {
    }

    void updateForce(Particle* particle, double t) override {
        if (remaining <= 0.0) return;
        particle->AddForce(force);
        remaining -= t;
    }

    bool isExpired() const { return remaining <= 0.0; }

private:
    Vector3 force;
    double remaining;
};
