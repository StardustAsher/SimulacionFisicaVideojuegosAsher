#include "ParticleGenerator.h"
#include <algorithm> // std::min, std::max

// Constructor
ParticleGenerator::ParticleGenerator(Vector3 position,
    Vector3 meanVelocity,
    Vector3 gravity,
    double emitRate,
    double particleLifetime,
    int shape,
    Vector4 baseColor,
    bool useGaussian,
    double var,
    double colorVar,
    double alphaVar,
    double sizeVar,
    double speedVar)
    : position(position),
    meanVelocity(meanVelocity),
    gravity(gravity),
    emitRate(emitRate),
    particleLifetime(particleLifetime),
    shape(shape),
    baseColor(baseColor),
    useGaussian(useGaussian),
    variance(var),
    colorVar(colorVar),
    alphaVar(alphaVar),
    sizeVar(sizeVar),
    speedVar(speedVar)
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
    // Velocidad, color y tamaño aleatorios
    Vector3 vel = randomVelocity();
    Vector4 color = randomColor();
    double size = randomSize();

    
    vel = (meanVelocity + vel) * (1.0 + speedVar * uniform(generator));

    // Crear partícula
    Particle* p = new Particle(position, vel, gravity, particleLifetime, 1.0, 0.99, shape, color);

    particleList.push_back(p);
}

Vector3 ParticleGenerator::randomVelocity() {
    auto rnd = [&](double base) {
        return base + (useGaussian ? gaussian(generator) : uniform(generator)) * variance;
        };
    return Vector3(rnd(0), rnd(0), rnd(0));
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
    double s = 1.0 + uniform(generator) * sizeVar;
    if (s < 0.1) s = 0.1; // evitar tamaños demasiado pequeños
    return s;
}
