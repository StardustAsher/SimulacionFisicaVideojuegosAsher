#include "Particle.h"
#include <iostream>
#include <fstream>

Particle::Particle(Vector3 pos, Vector3 vel, Vector3 a, double lt, double m, double d, int shape, Vector4 color, double size)
{
	//Inicializar variables
	position = physx::PxTransform(pos);
	prevPosition = pos;
	velocity = vel;
	acceleration = a;
	lifeTime = lt;
	damping = d;
	if (m <= 0) mass = 0.0;
	else mass = m;
	_shape = shape;
	firstIntegrate = true;
	accumulatedForce = Vector3(0.0f, 0.0f, 0.0f);
	particleSize = size;

	if (std::isnan(particleSize) || particleSize <= 0.0)
		particleSize = 0.1;  // tamaño mínimo


	// Shapes
	physx::PxGeometry* geometry = nullptr;
	switch (shape) {
	case 1: // Esfera
		geometry = new physx::PxSphereGeometry(particleSize);
		break;
	case 2: // Cubo
		geometry = new physx::PxBoxGeometry(particleSize, particleSize, particleSize);
		break;
	case 3: // Cilindro
		geometry = new physx::PxCapsuleGeometry(particleSize * 0.2, particleSize);
		break;
	default: // Por defecto
		geometry = new physx::PxSphereGeometry(particleSize);
		break;
	}


	// Render
	renderItem = new RenderItem(
		CreateShape(*geometry),   
		&position,
		color                     
	);

	//RegisterRenderItem(renderItem);

	
	delete geometry;
}

Particle::~Particle()
{
	DeregisterRenderItem(renderItem);
	delete renderItem;
}

void Particle::integrate(double t)
{
	if (mass <= 0.0 || lifeTime <= 0.0) return;

	lifeTime -= t;

	acceleration = accumulatedForce * getInvMass();
	////Euler
	//if (mass > 0.0 && lifeTime > 0.0) {

	//	lifeTime -= t;

	//	position.p = position.p + velocity * t;
	//	velocity = velocity * pow(damping, t) + accumulatedForce * t;
	//	
	//}

	//Euler semi-implicito
	if(mass> 0.0 && lifeTime > 0.0 ) {
		
		velocity = velocity * pow(damping, t) + acceleration * t;
		position.p = position.p + velocity * t;

		prevPosition = position.p;
		firstIntegrate = false;
		
		
	}

	//Verlet
	/*else if (mass > 0.0 && lifeTime > 0.0 ) {

		lifeTime -= t;

		Vector3 currentPos = position.p;

		Vector3 newPos = 2.0f * currentPos - prevPosition + acceleration * (t * t);

		std::cout << "New Pos: " << newPos.x << ", " << newPos.y << ", " << newPos.z << std::endl;

		prevPosition = currentPos;
		position.p = newPos;
	}*/

	ClearForce();

}
