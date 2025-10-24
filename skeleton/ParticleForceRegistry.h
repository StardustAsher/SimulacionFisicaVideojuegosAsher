#pragma once
#include <vector>
#include "ForceGenerator.h"
#include "Particle.h"


class ParticleForceRegistry
{
    protected:
        struct ForceRegistration {
            Particle* particle;
            ForceGenerator* fg;
        };

        std::vector<ForceRegistration> registrations;

    public:
        void add(Particle* particle, ForceGenerator* fg);
        void remove(Particle* particle, ForceGenerator* fg);
        void clear();
        void updateForces(double t);
    
};

