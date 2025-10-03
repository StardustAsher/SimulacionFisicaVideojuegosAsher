#pragma once
#include "RenderUtils.hpp"

class Particle
{
public:
	Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d, int shape, Vector4 color);
	~Particle();

	void integrate(double t);
	bool isAlive() { return lifeTime > 0.0; }
	

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
};

