#include "GravityForceGenerator.h"

GravityForceGenerator::GravityForceGenerator(const Vector3& g)
{
	gravity = g;

}

void GravityForceGenerator::updateForce(Particle* p, double t)
{
    if (p->getMass() <= 0.0) return;

    Vector3 force(0.0, gravity.y / p->getInvMass(), 0.0);

    p->AddForce(force);
}
