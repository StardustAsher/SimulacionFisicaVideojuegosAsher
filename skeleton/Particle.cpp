#include "Particle.h"

Particle::Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d, int shape, Vector4 color)
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
	_shape = shape;


	// Shapes
	physx::PxGeometry* geometry = nullptr;
	switch (shape) {
	case 1: // Esfera
		geometry = new physx::PxSphereGeometry(0.5);
		break;
	case 2: // Cubo
		geometry = new physx::PxBoxGeometry(0.5, 0.5, 0.5);
		break;
	case 3: // Cilinro
		geometry = new physx::PxCapsuleGeometry(0.2, 1.0); 
		break;
	default: // Por defecto
		geometry = new physx::PxSphereGeometry(0.5);
		break;
	}

	// Render
	renderItem = new RenderItem(
		CreateShape(*geometry),   
		&position,
		color                     
	);

	RegisterRenderItem(renderItem);

	
	delete geometry;
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

