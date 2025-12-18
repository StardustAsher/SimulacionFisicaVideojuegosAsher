#pragma once

#include "ForceGenerator.h"
#include "core.hpp"

class FloatForceGenerator : public ForceGenerator {
public:
    FloatForceGenerator(float h, float V, float d, Particle* liquid);

    virtual void updateForce(Particle* particle, double duration) override;

    virtual ~FloatForceGenerator();

protected:
    float _height;
    float _volume;
    float _liquid_density;
    float _gravity = 9.8f;

    Particle* _liquid_particle;
};
