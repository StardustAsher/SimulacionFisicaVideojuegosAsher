#include "FloatForceGenerator.h"
#include "Particle.h"

FloatForceGenerator::FloatForceGenerator(float h, float V, float d, Particle* liquid)
    : _height(h), _volume(V), _liquid_density(d), _liquid_particle(liquid) {
}

FloatForceGenerator::~FloatForceGenerator() {}

void FloatForceGenerator::updateForce(Particle* p, double /*duration*/) {
    if (!_liquid_particle || p->getMass() <= 0.0) return;

    float h = p->getPositionVector().y;
    float h0 = _liquid_particle->getPositionVector().y;

    Vector3 f(0, 0, 0);
    float immersed = 0.0f;

    if (h - h0 > _height * 0.5f) {
        immersed = 0.0f;
    }
    else if (h0 - h > _height * 0.5f) {
        immersed = 1.0f;
    }
    else {
        immersed = (h0 - h) / _height + 0.5f;
    }

    f.y = _liquid_density * _volume * immersed * _gravity;
    p->AddForce(f);
}
