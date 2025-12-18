#pragma once
#include <vector>
#include <PxPhysicsAPI.h>
#include "RigidForceGenerator.h"

class RigidForceRegistry
{
protected:
    struct ForceRegistration {
        physx::PxRigidDynamic* body;   // Sólido rígido
        RigidForceGenerator* fg;       // Generador de fuerza
    };

    std::vector<ForceRegistration> registrations;

public:
    // Registrar un generador de fuerza sobre un cuerpo rígido
    void add(physx::PxRigidDynamic* body, RigidForceGenerator* fg) {
        registrations.push_back({ body, fg });
    }

    // Eliminar un generador de fuerza
    void remove(physx::PxRigidDynamic* body, RigidForceGenerator* fg) {
        for (auto it = registrations.begin(); it != registrations.end(); ++it) {
            if (it->body == body && it->fg == fg) {
                registrations.erase(it);
                return;
            }
        }
    }

    // Limpiar todo el registro
    void clear() {
        registrations.clear();
    }

    // Actualizar fuerzas de todos los cuerpos rígidos
    void updateForces(double t) {
        for (auto& reg : registrations) {
            if (reg.body && reg.fg) {
                reg.fg->updateForce(reg.body, t);
            }
        }
    }

    // Opcional: actualizar fuerzas condicionales
    void updateForcesConditional(double t, bool gravedadActiva, bool vientoActivo, RigidForceGenerator* gravity, RigidForceGenerator* wind) {
        for (auto& reg : registrations) {
            if (!reg.body || !reg.fg) continue;

            RigidForceGenerator* fg = reg.fg;

            // Solo aplicar fuerzas activas
            if ((gravedadActiva && fg == gravity) || (vientoActivo && fg == wind) || (fg != gravity && fg != wind)) {
                fg->updateForce(reg.body, t);
            }
        }
    }

    // Podrías implementar removeExpired si tus RigidForceGenerators tienen duración limitada
    void removeExpired(double t) {
        //for (auto it = registrations.begin(); it != registrations.end();) {
        //    // Ejemplo: si RigidForceGenerator tuviera un método isExpired(t)
        //    if (it->fg /* && it->fg->isExpired(t) */) {
        //        it = registrations.erase(it);
        //    }
        //    else {
        //        ++it;
        //    }
        //}
    }
};
