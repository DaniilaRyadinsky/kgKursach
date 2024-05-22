#ifndef RAY_H
#define RAY_H

#include "MyVector3d.h"

//луч
class Ray
{
public:
	//точка начала
	Vec3 origin;

	//напрввление
	Vec3 direction;
};

#endif