#include "ParticleForceRegistry.h"
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
