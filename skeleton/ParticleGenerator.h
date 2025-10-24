#pragma once

#include "Particle.h"
#include <vector>
#include <random>
#include <algorithm> // std::min, std::max

class ParticleGenerator {
public:
    ParticleGenerator(Vector3 position,
        Vector3 meanVelocity,
        int gravity,
        double emitRate = 10.0,
        double particleLifetime = 5.0,
        int shape = 0,
        Vector4 baseColor = Vector4(1, 1, 1, 1),
        bool useGaussian = false,
        double var = 1.0,
        double colorVar = 0.0,
        double alphaVar = 0.0,
        double sizeVar = 0.0,
        double speedVar = 0.0);

    // Actualiza el emisor y genera nuevas partículas
    void update(double t, std::vector<Particle*>& particleList);

private:
    // Propiedades base
    Vector3 position;
    Vector3 meanVelocity;
    Vector3 gravity;
    double emitRate;
    double particleLifetime;
    int shape;
    Vector4 baseColor;
    int gravityType; // 0 = none, 1 = earth, 2 = moon

    // Control de variabilidad
    bool useGaussian;
    double variance;     // variación direccional básica
    double colorVar;     // variación del color RGB
    double alphaVar;     // variación de la opacidad
    double sizeVar;      // variación del tamaño
    double speedVar;     // variación de la velocidad

    double timeAccumulator = 0.0;

    // Distribuciones aleatorias
    std::default_random_engine generator;
    std::uniform_real_distribution<double> uniform{ -1.0, 1.0 };
    std::normal_distribution<double> gaussian{ 0.0, 1.0 };

    // Métodos internos
    void emitParticle(std::vector<Particle*>& particleList);
    Vector3 randomVelocity();
    Vector4 randomColor();
    double randomSize();
};