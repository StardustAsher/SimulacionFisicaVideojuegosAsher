#include "WindForceGenerator.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

WindForceGenerator::WindForceGenerator(const Vector3& dir, double s)
    : baseDirection(dir.getNormalized()), strength(s)
{

}

void WindForceGenerator::updateForce(Particle* particle, double t)
{

	//acceder a todas las particulas que contengan esta fuerza y actualizarlas
	if (particle->getMass() <= 0.0f) return;

	//la velocidad del viento cambia en funcion del tiempo
	double oscillation = sin(t * 0.5) * 0.5 + 1.0;
	Vector3 windRealVel = (baseDirection * strength) * oscillation;

	Vector3 diff = windRealVel - particle->getVelocity();
	double module = diff.magnitude();

	// Evitar aplicar fuerza si la velocidad ya es casi la del viento
	if (module < 0.1) return;

	Vector3 windForce = k1 * diff + k2 * module * diff;
	particle->AddForce(windForce);


}

