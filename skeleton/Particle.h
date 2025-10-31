#pragma once
#include "RenderUtils.hpp"

class Particle
{
public:
	Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d, int shape, Vector4 color,double size);
	~Particle();

	void integrate(double t);
	bool isAlive() { return lifeTime > 0.0; }

	 void AddForce(const Vector3& force) {
        accumulatedForce += force;
    }

    void ClearForce() {
        accumulatedForce = {0, 0, 0};
    }

    double getMass() const { return mass > 0 ? 1.0 / mass : 0.0; }
    double getInvMass() const { return 1/mass; }
	

private:
	Vector3 velocity;
	Vector3 acceleration;
	double mass;
	double lifeTime;
	double damping;
	int _shape;
	physx:: PxTransform position;
	Vector3 prevPosition;
	RenderItem* renderItem;
	bool firstIntegrate = true;
	double particleSize;
	Vector3 accumulatedForce;
};

