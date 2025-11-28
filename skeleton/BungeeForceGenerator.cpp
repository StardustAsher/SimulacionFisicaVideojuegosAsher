#include "BungeeForceGenerator.h"
#include <cmath>

void BungeeForceGenerator::updateForce(Particle* particle, double /*t*/)
{
    if (!particle || !otherParticle) return;

    Vector3 posA = particle->getPositionVector();
    Vector3 posB = otherParticle->getPositionVector();

    Vector3 delta = posA - posB;
    double dist = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if (dist <= restLength) return; // la goma no aplica si no está estirada

    double magnitude = -k * (dist - restLength);
    Vector3 force = delta * (magnitude / dist);
    particle->AddForce(force);
}
