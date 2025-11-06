#pragma once
#include "ForceGenerator.h"
#include <cmath>

class WindForceGenerator : public ForceGenerator
{
private:
    Vector3 baseDirection;  // Dirección media del viento
    double strength;        // Intensidad base del viento
	double k1 = 0.3;        // Coeficiente de fuerza lineal
	double k2 = 0.0;       // Coeficiente de fuerza cuadrática

public:
    WindForceGenerator(const Vector3& dir, double strength);

    void updateForce(Particle* p, double t) override;
};
