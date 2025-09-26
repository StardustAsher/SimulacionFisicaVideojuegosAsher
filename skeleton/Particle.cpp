#include "Particle.h"

Particle::Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d)
{
	//Inicializar variables
	position = physx::PxTransform(pos);
	prevPosition = { pos.x - vel.x, pos.y - vel.y, pos.z - vel.z };
	velocity = vel;
	acceleration = a;
	lifeTime = lt;
	damping = d;
	if (m <= 0) mass = 0.0;
	else mass = 1.0 / m;



	// Render
	renderItem = new RenderItem(
		CreateShape(physx::PxSphereGeometry(0.5)),   
		&position,
		Vector4(0.8, 0.1, 0.1, 1.0)                 
	);
	RegisterRenderItem(renderItem);
}

Particle::~Particle()
{
	DeregisterRenderItem(renderItem);
	//delete renderItem;
}

void Particle::integrate(double t)
{
	/*
	//Euler
	if (mass > 0.0 && lifeTime > 0.0) {

		lifeTime -= t;

		position.p = position.p + velocity * t;
		velocity = velocity * pow(damping, t) + acceleration * t;
		
	}

	//Euler semi-implicito
	if(mass> 0.0 && lifeTime > 0.0) {
		
		lifeTime -= t;

		velocity = velocity * pow(damping, t) + acceleration * t;
		position.p = position.p + velocity * t;
	}

	*/


	//Verlet
	if (mass > 0.0 && lifeTime > 0.0) {

		lifeTime -= t;

		Vector3 currentPos = position.p;

		Vector3 newPos = currentPos + (currentPos - prevPosition) * pow(damping, t) + acceleration * (t * t);

		prevPosition = currentPos;
		position.p = newPos;

	}



}

