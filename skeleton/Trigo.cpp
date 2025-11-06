#include "Trigo.h"
#include "core.hpp"
#include "RenderUtils.hpp"
#include "PxPhysicsAPI.h"
#include <limits>

extern physx::PxPhysics* gPhysics;
extern physx::PxMaterial* gMaterial;

Trigo::Trigo(Vector3 posBase) : posicionBase(posBase) {
    numCubosTotales = 8;
    alturaCubo = 0.7f;
    progresoCrecimiento = 0.0f;
    cubosCreados = 0;
    progresoMaduracion = 0.0f;
    madurando = false;

    particulas = new ParticleGenerator(
        posBase + Vector3(0, 1.0f, 0),     
		Vector3(0, 5.0f, 0),              // velocidad inicial hacia arriba
        0,                                 // SIN gravedad (tipo 0 = sin registro en forceRegistry)
        20.0,                               // tasa de emisión (partículas por segundo)
        0.5,                               // vida 
        2,                                 // forma cuadrada
        Vector4(1.0f, 1.0f, 0.1f, 0.8f),   // color base
        0.1f,                              // tamaño base 
        false,                             // sin gaussiano
        0.0,                              // varianza dir
        0.1,                               // variación de color leve
        0.1,                               // variación de opacidad
        0.1,                               // variación de tamaño
        0.5,                                 // variación de tamaño
        3, 0.0, 3                  // varx, vary, varz
    );
    particulas->setActive(false);
}

Trigo::~Trigo() {
    delete particulas;
    particulas = nullptr;

    for (auto& t : tallos)
        delete t;
    tallos.clear();

    
}

void Trigo::regar() {
    regando = true;
    tiempoRegando = 1.1f;
}

void Trigo::update(double t) {
    // Fase 1: Crecimiento
    if (regando && !madurando) {
        progresoCrecimiento += t * 0.4f;
        if (progresoCrecimiento > 1.0f) progresoCrecimiento = 1.0f;

        int cubosEsperados = static_cast<int>(progresoCrecimiento * numCubosTotales);
        while (cubosCreados < cubosEsperados) {
            float offsetX = (cubosCreados % 2 == 0) ? 0.12f : -0.12f;
            float y = cubosCreados * alturaCubo;

            physx::PxShape* shape = gPhysics->createShape(
                physx::PxBoxGeometry(alturaCubo / 2.5f, alturaCubo / 2.0f, alturaCubo / 2.5f),
                *gMaterial);

            physx::PxTransform* transform = new physx::PxTransform(
                physx::PxVec3(posicionBase.x + offsetX, posicionBase.y + y, posicionBase.z));

            Vector4 colorInicial(0.0f, 1.0f, 0.0f, 1.0f);
            RenderItem* cubito = new RenderItem(shape, transform, colorInicial);
            RegisterRenderItem(cubito);
            tallos.push_back(cubito);

            cubosCreados++;
        }

        tiempoRegando -= t;
        if (tiempoRegando <= 0.0f) regando = false;

        if (progresoCrecimiento >= 1.0f) {
            regando = false;
            madurando = true;
        }
    }

    // Fase 2: Maduración (solo cambio de color)
    if (madurando) {
        progresoMaduracion += t * 0.3f;
        if (progresoMaduracion > 1.0f) progresoMaduracion = 1.0f;

        for (auto& cubo : tallos) {
            float r = progresoMaduracion * 1.0f;
            float g = 1.0f - progresoMaduracion * 0.3f;
            float b = 0.0f;
            cubo->color = Vector4(r, g, b, 1.0f);
        }

        // cuando termina de madurar, activamos el generador
        if (progresoMaduracion >= 1.0f && !particulas->isActive())
            particulas->setActive(true);
    }
}
