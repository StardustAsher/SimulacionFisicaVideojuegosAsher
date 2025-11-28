#pragma once
#include "ForceGenerator.h"
#include "Particle.h"

// Muelle entre partícula y punto fijo (ancla)
class AnchoredSpringForceGenerator : public ForceGenerator {
public:
    AnchoredSpringForceGenerator(const Vector3& anchor, double k, double restLength)
        : anchor(anchor), k(k), restLength(restLength) {
    }

    void updateForce(Particle* particle, double t) override;

    void setK(double k_) { k = k_; }
    double getK() const { return k; }
    void setAnchor(const Vector3& a) { anchor = a; }
private:
    Vector3 anchor;
    double k;
    double restLength;
};
