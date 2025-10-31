#include "ParticleGenerator.h"
#include <algorithm> // std::min, std::max
#include "ParticleForceRegistry.h"
#include "GravityForceGenerator.h"

ParticleForceRegistry forceRegistry;
extern GravityForceGenerator* gravityEarth;
extern GravityForceGenerator* gravityMoon;


// Constructor
ParticleGenerator::ParticleGenerator(Vector3 position,
    Vector3 meanVelocity,
    int gravity,
    double emitRate,
    double particleLifetime,
    int shape,
    Vector4 baseColor,
	double size,
    bool useGaussian,
    double var,
    double colorVar,
    double alphaVar,
    double sizeVar,
    double speedVar,double varx,double vary,double varz)
    : position(position),
    meanVelocity(meanVelocity),
    emitRate(emitRate),
    particleLifetime(particleLifetime),
    shape(shape),
    baseColor(baseColor),
	baseSize(size),
    useGaussian(useGaussian),
    variance(var),
    colorVar(colorVar),
    alphaVar(alphaVar),
    sizeVar(sizeVar),
    speedVar(speedVar),
	gravityType(gravity),varx(varx), vary(vary), varz(varz)
{
}

void ParticleGenerator::update(double t, std::vector<Particle*>& particleList) {
    timeAccumulator += t;
    double interval = 1.0 / emitRate;

    while (timeAccumulator >= interval) {
        timeAccumulator -= interval;
        emitParticle(particleList);
    }
}

void ParticleGenerator::emitParticle(std::vector<Particle*>& particleList) {

    Vector3 vel = randomVelocity();
    Vector4 color = randomColor();
    double size = randomSize();

    vel = (meanVelocity + vel) * (1.0 + speedVar * uniform(generator));

    // Crear part�cula 
    Particle* p = new Particle(position, vel, Vector3(0, 0, 0), particleLifetime, 10.0, 0.99, shape, color,size);
    particleList.push_back(p);

    // Registrar gravedad seg�n tipo
    if (gravityType == 1) forceRegistry.add(p, gravityEarth);
    else if (gravityType == 2) forceRegistry.add(p, gravityMoon);
}


Vector3 ParticleGenerator::randomVelocity() {
    // Variaci�n independiente por componente
    auto rnd = [&](double var) {
        return (useGaussian ? gaussian(generator) : uniform(generator)) * var;
    };

    // meanVelocity define la direcci�n base del chorro
    double vx = meanVelocity.x + rnd(varx);
    double vy = meanVelocity.y + rnd(vary);
    double vz = meanVelocity.z + rnd(varz);

    return Vector3(vx, vy, vz);
}



Vector4 ParticleGenerator::randomColor() {
	// Limitar entre 0 y 1
    auto limit = [](float v) {
        if (v < 0.0f) return 0.0f;
        if (v > 1.0f) return 1.0f;
        return v;
        };

    float r = limit(baseColor.x + uniform(generator) * colorVar);
    float g = limit(baseColor.y + uniform(generator) * colorVar);
    float b = limit(baseColor.z + uniform(generator) * colorVar);
    float a = limit(baseColor.w + uniform(generator) * alphaVar);

    return Vector4(r, g, b, a);
}

double ParticleGenerator::randomSize() {
    // tama�o base definido en el constructor
    double s = baseSize + uniform(generator) * sizeVar;

    // l�mite inferior para evitar part�culas invisibles
    if (s < 0.1) s = 0.1;

    return s;
}

