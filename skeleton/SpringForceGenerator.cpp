#include "SpringForceGenerator.h"
#include <cmath>

void SpringForceGenerator::updateForce(Particle* particle, double t)
{
    if (!particle || !otherParticle) return;

    Vector3 posA = particle->getPositionVector();
    Vector3 posB = otherParticle->getPositionVector();

    Vector3 delta = posA - posB;
    double dist = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (dist <= 0.000001) return;

    double magnitude = -k * (dist - restLength);
    Vector3 force = delta * (magnitude / dist); // dirección desde B hacia A
    particle->AddForce(force);
}
