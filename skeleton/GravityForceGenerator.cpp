#include "GravityForceGenerator.h"

GravityForceGenerator::GravityForceGenerator(const Vector3& g)
{
	gravity = g;

}

void GravityForceGenerator::updateForce(Particle* p, double t)
{
    if (p->getMass() <= 0.0) return;

    Vector3 force = gravity * (1.0 / p->getInvMass());
    p->AddForce(force);
}
