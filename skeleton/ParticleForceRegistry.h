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
        
        void updateForcesConditional(double t, bool gravedadActiva, bool vientoActivo, ForceGenerator* gravity, ForceGenerator* wind) {
            for (auto& reg : registrations) {
                if ((gravedadActiva && reg.fg == gravity) ||
                    (vientoActivo && reg.fg == wind)) {
                    reg.fg->updateForce(reg.particle, t);
                }
            }
        }
    
};

