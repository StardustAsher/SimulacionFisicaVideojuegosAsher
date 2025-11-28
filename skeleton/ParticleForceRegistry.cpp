#include "ParticleForceRegistry.h"
#include "TimeForceGenerator.h"
#include <algorithm> 
#include <iostream>


void ParticleForceRegistry::add(Particle* particle, ForceGenerator* fg) // añadir al registro la pareja particula-fuerza
{
    registrations.push_back({ particle, fg });
	
}

void ParticleForceRegistry::remove(Particle* particle, ForceGenerator* fg) //eliminar el un registro particula-fuerza
{
    registrations.erase(
        std::remove_if(registrations.begin(), registrations.end(),
            [particle, fg](const ForceRegistration& r) {
                return r.particle == particle && r.fg == fg;
            }), registrations.end());

}

void ParticleForceRegistry::clear() //limpiar el registro
{
    registrations.clear();
}

void ParticleForceRegistry::updateForces(double t) //llamar a todos los updateForce
{
    for (auto& reg : registrations) {
        reg.fg->updateForce(reg.particle, t);
    }

}

void ParticleForceRegistry::removeExpired(double t)
{
    for (int i = (int)registrations.size() - 1; i >= 0; --i)
    {
        ForceGenerator* fg = registrations[i].fg;

        if (fg->getType() == ForceGenerator::TIMED)
        {
            
            TimedForceGenerator* tf = static_cast<TimedForceGenerator*>(fg);

            tf->updateForce(registrations[i].particle, t);

            if (tf->isExpired()) {
                delete tf;
                registrations.erase(registrations.begin() + i);
            }
        }
    }
}

void ParticleForceRegistry::updateForcesConditional(
    double t,
    bool gravedadActiva,
    bool vientoActivo,
    ForceGenerator* gravity,
    ForceGenerator* wind)
{
    for (auto& reg : registrations) {
        // Gravedad
        if (reg.fg == gravity) {
            if (gravedadActiva)
                reg.fg->updateForce(reg.particle, t);
            continue;
        }

        // Viento
        if (reg.fg == wind) {
            if (vientoActivo)
                reg.fg->updateForce(reg.particle, t);
            continue;
        }

        // Todo lo demás (muelles, etc.) siempre se actualiza
        reg.fg->updateForce(reg.particle, t);
    }
}


