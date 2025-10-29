#include "Particle.h"
#include <iostream>
#include <fstream>

Particle::Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d, int shape, Vector4 color)
{
	//Inicializar variables
	position = physx::PxTransform(pos);
	prevPosition = pos;
	velocity = vel;
	acceleration = a;
	lifeTime = lt;
	damping = d;
	if (m <= 0) mass = 0.0;
	else mass = 1.0 / m;
	_shape = shape;
	firstIntegrate = true;


	// Shapes
	physx::PxGeometry* geometry = nullptr;
	switch (shape) {
	case 1: // Esfera
		geometry = new physx::PxSphereGeometry(1.0);
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
	acceleration = accumulatedForce / mass;

	std::cout << "Particle Acceleration: " << acceleration.x << ", " << acceleration.y << ", " << acceleration.z << std::endl;
	std::cout << "Particle Velocity: " << velocity.x << ", " << velocity.y << ", " << velocity.z << std::endl;
	std::cout << "Acumulated Force: " << accumulatedForce.x << ", " << accumulatedForce.y << ", " << accumulatedForce.z << std::endl;
	////Euler
	//if (mass > 0.0 && lifeTime > 0.0) {

	//	lifeTime -= t;

	//	position.p = position.p + velocity * t;
	//	velocity = velocity * pow(damping, t) + accumulatedForce * t;
	//	
	//}

	//Euler semi-implicito
	if(mass> 0.0 && lifeTime > 0.0 && firstIntegrate) {
		
		lifeTime -= t;

		velocity = velocity * pow(damping, t) + acceleration * t;
		position.p = position.p + velocity * t;
		firstIntegrate = false;
	}

	


	//Verlet
	else if (mass > 0.0 && lifeTime > 0.0 ) {

		lifeTime -= t;

		Vector3 currentPos = position.p;

		Vector3 newPos = 2.0f * currentPos - prevPosition + acceleration * (t * t);

		prevPosition = currentPos;
		position.p = newPos;
	}

	std::cout << "Particle Position: " << position.p.x << ", " << position.p.y << ", " << position.p.z << std::endl;

	ClearForce();

}
