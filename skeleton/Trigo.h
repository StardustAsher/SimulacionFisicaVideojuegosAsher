#pragma once
#include <vector>
#include "RenderUtils.hpp"
#include "ParticleGenerator.h"
#include "Vector3D.h"
#include <list>

class Trigo {
private:

    std::list<RenderItem*> tallos;
    //std::vector<RenderItem*> tallos;
    ParticleGenerator* particulas = nullptr;
    // Ya no hay vector interno de partículas: las usaremos globalmente

    Vector3 posicionBase;

    // Variables de crecimiento
    float progresoCrecimiento = 0.0f; // entre 0 y 1
    bool regando = false;
    float tiempoRegando = 0.0f;

    // Variables de maduración (color)
    bool madurando = false;
    float progresoMaduracion = 0.0f; // entre 0 y 1

    // Control de estructura
    int numCubosTotales = 0;
    int cubosCreados = 0;
    float alturaCubo = 0.0f;

public:
    Trigo(Vector3 posBase);
    ~Trigo();

    void update(double t);
    void regar(); // activa el crecimiento
    void reset();
    bool estaMaduro() const;


    // getter para acceder al generador desde el main
    ParticleGenerator* getParticleGenerator() const { return particulas; }
};
