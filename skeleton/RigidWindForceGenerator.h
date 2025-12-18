#pragma once
#include "RigidForceGenerator.h"
#include <PxPhysicsAPI.h>
#include "Vector3D.h"

class RigidWindForceGenerator : public RigidForceGenerator
{
private:
    Vector3D direction;   // Dirección del viento (normalizada)
    double strength;      // Intensidad del viento
    double k1 = 0.3;      // Coeficiente lineal
    double k2 = 0.0;      // Coeficiente cuadrático (opcional)

public:
    RigidWindForceGenerator(const Vector3D& dir, double s)
        : RigidForceGenerator(GENERIC), strength(s)
    {
        direction = dir;
        direction.normalize(); 
    }

    void updateForce(physx::PxRigidDynamic* body, double t) override
    {
        PX_UNUSED(t);
        if (!body) return;

        // Velocidad actual del cuerpo
        physx::PxVec3 v = body->getLinearVelocity();
        Vector3D vel(v.x, v.y, v.z);

        // Velocidad del viento 
        Vector3D windVel = direction * strength;

        // Diferencia viento - velocidad del cuerpo
        Vector3D diff = windVel - vel;
        double module = diff.magnitude();
        if (module < 0.01) return;

        // Fuerza del viento
        Vector3D windForce(
            k1 * diff.x + k2 * module * diff.x,
            k1 * diff.y + k2 * module * diff.y,
            k1 * diff.z + k2 * module * diff.z
        );

        // Aplicar fuerza como ACCELERACIÓN
        body->addForce(
            physx::PxVec3(
                (float)windForce.x,
                (float)windForce.y,
                (float)windForce.z
            ),
            physx::PxForceMode::eACCELERATION
        );
    }
};
