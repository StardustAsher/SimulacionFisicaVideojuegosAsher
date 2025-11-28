#include "AnchoredSpringForceGenerator.h"
#include <cmath>

void AnchoredSpringForceGenerator::updateForce(Particle* particle, double t)
{
    if (!particle) return;

    Vector3 pos = particle->getPositionVector();
    Vector3 delta = pos - anchor;
    double dist = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (dist <= 0.000001) return;

    double magnitude = -k * (dist - restLength);
    Vector3 force = delta * (magnitude / dist);
    particle->AddForce(force);
}
